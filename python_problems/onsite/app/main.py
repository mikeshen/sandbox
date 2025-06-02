from fastapi import FastAPI, HTTPException
from models import User

app = FastAPI()

# Example in-memory user store
users = [
    User(id=1, name="Alice", email="alice@example.com"),
    User(id=2, name="Bob", email="bob@example.com")
]

@app.get("/")
def root():
    return {
        "available_endpoints": [
            {"path": "/user", "method": "GET", "description": "Get a sample user"}
        ]
    }

@app.get("/user", response_model=User)
def get_user(user_id: int):
    for user in users:
        if user.id == user_id:
            return user
    raise HTTPException(status_code=404, detail="User not found")
