import requests
import pymongo
import json
import time

BASE_URL = "http://localhost:8080"
MONGO_URI = "mongodb://localhost:27017"
DB_NAME = "testdb" # Default in Database.hpp

def main():
    # 1. Connect to MongoDB
    client = pymongo.MongoClient(MONGO_URI)
    db = client[DB_NAME]
    jobs_col = db["jobs"]
    
    print(f"Connected to MongoDB: {DB_NAME}")

    # 2. Register Company
    company_email = f"test_company_{int(time.time())}@example.com"
    company_data = {
        "name": "Test Company",
        "email": company_email,
        "password": "Password123!",
        "isCompany": True
    }
    
    print(f"Registering company: {company_email}")
    res = requests.post(f"{BASE_URL}/auth/register/company", json=company_data)
    if res.status_code != 200:
        print(f"Registration failed: {res.text}")
        return

    # 3. Login
    login_data = {
        "email": company_email,
        "password": "Password123!"
    }
    res = requests.post(f"{BASE_URL}/auth/login", json=login_data)
    if res.status_code != 200:
        print(f"Login failed: {res.text}")
        return
    
    token = res.json().get("token")
    headers = {"Authorization": f"Bearer {token}"}
    print("Logged in.")

    # 4. Create Job
    job_data = {
        "title": "Software Engineer",
        "description": "Develop software",
        "requiredLanguages": ["C++", "Python"],
        "grade": "Junior",
        "skills": ["Linux", "Git"],
        "category": "Backend",
        "salaryRange": {
            "min": 50000,
            "max": 80000
        },
        "location": "Remote",
        "workType": ["Full-time"]
    }
    
    print("Creating job...")
    res = requests.post(f"{BASE_URL}/jobs", json=job_data, headers=headers)
    if res.status_code != 200:
        print(f"Create job failed: {res.text}")
        return
    
    job_id = res.json().get("_id")
    print(f"Job created: {job_id}")

    # 5. Inspect in DB (POST)
    job_doc = jobs_col.find_one({"_id": pymongo.ObjectId(job_id)})
    salary_range = job_doc.get("salaryRange")
    print(f"\n[POST] salaryRange type: {type(salary_range)}")
    print(f"[POST] salaryRange value: {salary_range}")

    # 6. Update Job
    update_data = {
        "salaryRange": {
            "min": 60000,
            "max": 90000
        }
    }
    
    print("\nUpdating job...")
    res = requests.put(f"{BASE_URL}/jobs/{job_id}", json=update_data, headers=headers)
    if res.status_code != 200:
        print(f"Update job failed: {res.text}")
        return

    # 7. Inspect in DB (PUT)
    job_doc = jobs_col.find_one({"_id": pymongo.ObjectId(job_id)})
    salary_range = job_doc.get("salaryRange")
    print(f"\n[PUT] salaryRange type: {type(salary_range)}")
    print(f"[PUT] salaryRange value: {salary_range}")

if __name__ == "__main__":
    main()
