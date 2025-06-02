from fastapi.testclient import TestClient
from app.main import app

client = TestClient(app)

def test_read_root():
    response = client.get("/")
    assert response.status_code == 200
    assert response.json() == {
        "available_endpoints": [
            {"path": "/user", "method": "GET", "description": "Get a sample user"}
        ]
    }

def test_get_user_found():
    response = client.get("/user", params={"user_id": 1})
    assert response.status_code == 200
    assert response.json() == {"id": 1, "name": "Alice", "email": "alice@example.com"}

    response = client.get("/user", params={"user_id": 2})
    assert response.status_code == 200
    assert response.json() == {"id": 2, "name": "Bob", "email": "bob@example.com"}

def test_get_user_not_found():
    response = client.get("/user", params={"user_id": 999})
    assert response.status_code == 404
    assert response.json()["detail"] == "User not found"
