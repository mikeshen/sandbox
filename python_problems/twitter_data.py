"""
# Twitter Data Analysis

## Introduction
This notebook is dedicated to analyzing data from Twitter. The goal is to extract, clean, and analyze tweets from a specific user to gain insights into their behavior, preferences, and likely views. We will use various data analysis techniques and visualization tools to achieve this.

## Data Loading
In this section, we will use the `twikit` library to pull the latest tweets for a particular user. The `twikit` library provides an easy-to-use interface for accessing Twitter's API and retrieving tweets. You can find more information about the library [here](https://github.com/d60/twikit/tree/main).

## Data Cleaning
Data cleaning is a crucial step in the data analysis process. It involves removing or correcting any inaccuracies or inconsistencies in the data. For our Twitter data, we will perform the following cleaning steps:
1. Remove retweets and replies.
2. Remove any tweets containing URLs, as they may not provide meaningful content.
3. Normalize text by converting to lowercase and removing special characters.

"""

import asyncio
from fastapi import FastAPI
from fastapi.responses import HTMLResponse, JSONResponse
from twikit import Client
from dotenv import load_dotenv
import os
from pprint import pprint

# Load environment variables from .env file
load_dotenv()

USERNAME = os.getenv('TWITTER_USERNAME')
EMAIL = os.getenv('TWITTER_EMAIL')
PASSWORD = os.getenv('TWITTER_PASSWORD')

# Print username and email
print(f"Username: {USERNAME}")
print(f"Email: {EMAIL}")

client = Client('en-US')
app = FastAPI()

@app.on_event("startup")
async def startup_event():
    await client.login(
        auth_info_1=USERNAME,
        auth_info_2=EMAIL,
        password=PASSWORD,
        cookies_file="cookies.json"
    )

async def fetch_tweets(screen_name: str, count: int = 5):
    try:
        # Get user by screen name
        user = await client.get_user_by_screen_name(screen_name)
        
        # Get user tweets
        user_tweets = await user.get_tweets('Tweets', count=count)
        return user_tweets
    except Forbidden as e:
        print(f"Error: {e}")
        raise HTTPException(status_code=403, detail="Forbidden: Bad guest token")

@app.get("/", response_class=HTMLResponse)
async def default_page():
    return HTMLResponse(content="<html><body><h1>Welcome to the Twitter Data API</h1></body></html>")

@app.get("/read_tweets", response_class=JSONResponse)
async def read_tweets(screen_name: str, count: int = 5):
    user_tweets = await fetch_tweets(screen_name, count)
    
    # Pretty print tweet data
    tweets_data = []
    for tweet in user_tweets:
        tweet_data = {
            "Tweet ID": tweet.id,
            "Created At": tweet.created_at,
            "Text": tweet.text,
            "Language": tweet.lang,
            "Favorite Count": tweet.favorite_count,
            "Retweet Count": tweet.retweet_count,
            "User": tweet.user.screen_name
        }
        pprint(tweet_data)
        tweets_data.append(tweet_data)
    
    # Return tweet data as JSON response
    return JSONResponse(content=tweets_data)

@app.get("/user", response_class=JSONResponse)
async def get_user_data(screen_name: str):
    print("screen_name: ", screen_name)
    
    user = await client.get_user_by_screen_name(screen_name)
    # Pretty print user data
    user_data = {
        "user_id": user.id,
        "name": user.name,
        "screen_name": user.screen_name,
        "description": user.description,
        "followers_count": user.followers_count,
        "statuses_count": user.statuses_count,
        "location": user.location,
        "created_at": user.created_at
    }
    
    # Return user data as JSON response
    return JSONResponse(content=user_data)
    

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)