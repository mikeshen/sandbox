
import uuid
import hashlib
import time
import json
from datetime import datetime
import asyncio
import logging
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
from fastapi import FastAPI, HTTPException, Request
from pydantic import BaseModel
from typing import Optional, List

# Configure logging
logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Buffer configuration
BUFFER_SIZE = 1000  # Number of logs to buffer before writing
FLUSH_INTERVAL = 5  # Maximum seconds to wait before flushing buffer
PARQUET_FILE = 'audit_logs.parquet'

class ParquetAuditLogger:
    def __init__(self):
        self.buffer = []
        self.buffer_lock = asyncio.Lock()
        self.last_flush_time = time.time()
        
    async def initialize(self):
        """Initialize the Parquet file if it doesn't exist"""
        try:
            pq.read_table(PARQUET_FILE)
        except FileNotFoundError:
            # Create an empty Parquet file with the correct schema
            schema = pa.schema([
                ('event_id', pa.string()),
                ('user_id', pa.string()),
                ('timestamp', pa.timestamp('ms')),
                ('event_type', pa.string()),
                ('details', pa.string()),
                ('source_ip', pa.string()),
                ('hash', pa.string())
            ])
            empty_table = pa.Table.from_pandas(pd.DataFrame(columns=schema.names), schema=schema)
            pq.write_table(empty_table, PARQUET_FILE)
        
        # Start the background flush task
        asyncio.create_task(self._periodic_flush())
        
    async def _periodic_flush(self):
        """Periodically flush the buffer if it hasn't been flushed recently"""
        while True:
            await asyncio.sleep(1)  # Check every second
            current_time = time.time()
            if current_time - self.last_flush_time >= FLUSH_INTERVAL and self.buffer:
                await self.flush()
    
    def create_log_entry(self, user_id, event_type, details, source_ip):
        """Create a new audit log entry with tamper-proof hash"""
        timestamp = datetime.utcnow()
        event_id = uuid.uuid4()
        
        # Create a deterministic string for hashing
        raw_data = f"{event_id}|{user_id}|{timestamp.isoformat()}|{event_type}|{details}|{source_ip}"
        hash_value = hashlib.sha256(raw_data.encode()).hexdigest()
        
        return {
            'event_id': str(event_id),
            'user_id': user_id,
            'timestamp': timestamp,
            'event_type': event_type,
            'details': json.dumps(details) if isinstance(details, dict) else details,
            'source_ip': source_ip,
            'hash': hash_value
        }
    
    async def log_event(self, user_id, event_type, details, source_ip):
        """Add a log event to the buffer, flushing if buffer is full"""
        entry = self.create_log_entry(user_id, event_type, details, source_ip)
        
        async with self.buffer_lock:
            self.buffer.append(entry)
            if len(self.buffer) >= BUFFER_SIZE:
                await self.flush()
        
        return entry['event_id']
    
    async def flush(self):
        """Flush the buffer to Parquet"""
        async with self.buffer_lock:
            if not self.buffer:
                return
            
            buffer_to_flush = self.buffer.copy()
            self.buffer.clear()
        
        try:
            # Convert buffer entries to a DataFrame
            df = pd.DataFrame(buffer_to_flush)
            
            # Append to the Parquet file
            table = pa.Table.from_pandas(df)
            with pq.ParquetWriter(PARQUET_FILE, table.schema, use_dictionary=True, compression='snappy') as writer:
                writer.write_table(table)
            
            self.last_flush_time = time.time()
            logger.info(f"Flushed {len(buffer_to_flush)} log entries to Parquet")
        except Exception as e:
            logger.error(f"Error flushing logs to Parquet: {e}")
            # In a production system, you might want to implement retry logic
            # or store failed logs elsewhere to prevent data loss
    
    async def query_logs(self, user_id=None, start_time=None, end_time=None, 
                         event_type=None, limit=100, offset=0):
        """Query logs with various filters"""
        try:
            table = pq.read_table(PARQUET_FILE)
            df = table.to_pandas()
            
            if user_id:
                df = df[df['user_id'] == user_id]
                
            if start_time:
                df = df[df['timestamp'] >= start_time]
                
            if end_time:
                df = df[df['timestamp'] <= end_time]
                
            if event_type:
                df = df[df['event_type'] == event_type]
            
            # Apply ordering and limits
            df = df.sort_values(by='timestamp', ascending=False)
            df = df.iloc[offset:offset+limit]
            
            return df.to_dict(orient='records')
        except Exception as e:
            logger.error(f"Error querying logs from Parquet: {e}")
            return []

# FastAPI app and request models
app = FastAPI()

class LogRequest(BaseModel):
    user_id: str
    event_type: str
    details: dict

class QueryRequest(BaseModel):
    user_id: Optional[str] = None
    event_type: Optional[str] = None
    start_time: Optional[datetime] = None
    end_time: Optional[datetime] = None
    limit: Optional[int] = 100
    offset: Optional[int] = 0

logger_instance = ParquetAuditLogger()

@app.on_event("startup")
async def startup_event():
    logger.info("Starting up the application...")
    await logger_instance.initialize()
    logger.info("Logger initialized.")

@app.post("/log")
async def handle_log(request: LogRequest, req: Request):
    try:
        # Get client IP
        source_ip = req.client.host
        
        # Log the event
        event_id = await logger_instance.log_event(
            request.user_id,
            request.event_type,
            request.details,
            source_ip
        )
        
        return {"status": "success", "event_id": str(event_id)}
    
    except Exception as e:
        logger.error(f"Error handling log request: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/query")
async def handle_query(user_id: Optional[str] = None, event_type: Optional[str] = None,
                       start_time: Optional[datetime] = None, end_time: Optional[datetime] = None,
                       limit: Optional[int] = 100, offset: Optional[int] = 0):
    try:
        # Query the logs
        results = await logger_instance.query_logs(
            user_id=user_id,
            start_time=start_time,
            end_time=end_time,
            event_type=event_type,
            limit=limit,
            offset=offset
        )
        
        # Convert results to JSON-serializable format
        for result in results:
            if isinstance(result['event_id'], uuid.UUID):
                result['event_id'] = str(result['event_id'])
            if isinstance(result['timestamp'], datetime):
                result['timestamp'] = result['timestamp'].isoformat()
            
            # Try to parse JSON details if stored as string
            if isinstance(result['details'], str):
                try:
                    result['details'] = json.loads(result['details'])
                except:
                    pass  # Keep as string if not valid JSON
        
        return {
            "count": len(results),
            "results": results
        }
        
    except Exception as e:
        logger.error(f"Error handling query request: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/debug")
async def debug():
    return {"status": "running"}

@app.get("/debug/logger_status")
async def logger_status():
    return {
        "buffer_size": len(logger_instance.buffer),
        "last_flush_time": logger_instance.last_flush_time
    }

# Main function to run the server
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8080)
