#!/usr/bin/env python3
import requests
import json

BASE_URL = "http://localhost:8080"

print("Testing Admin Login...")
print("=" * 50)

# Test admin login
admin_data = {
    "email": "admin@gmail.com",
    "password": "A!1111"
}

response = requests.post(f"{BASE_URL}/auth/login", json=admin_data)
print(f"Status: {response.status_code}")
print(f"Response: {json.dumps(response.json(), indent=2)}")

if response.status_code == 200:
    data = response.json()
    if data.get("role") == "admin":
        print("\n✅ SUCCESS: Admin login working!")
        print(f"Admin Token: {data['token'][:50]}...")
        
        # Test admin endpoint
        print("\n" + "=" * 50)
        print("Testing Admin Endpoint (GET /admin/jobs/pending)...")
        
        admin_token = data['token']
        headers = {"Authorization": f"Bearer {admin_token}"}
        
        jobs_response = requests.get(f"{BASE_URL}/admin/jobs/pending", headers=headers)
        print(f"Status: {jobs_response.status_code}")
        print(f"Pending Jobs: {jobs_response.json()}")
        
        if jobs_response.status_code == 200:
            print("\n✅ SUCCESS: Admin can access admin endpoints!")
        else:
            print("\n❌ FAILED: Admin cannot access admin endpoints")
    else:
        print("\n❌ FAILED: Wrong role returned")
else:
    print("\n❌ FAILED: Admin login failed")
