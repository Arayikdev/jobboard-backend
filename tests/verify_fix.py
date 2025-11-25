import requests
import json
import time
import sys

BASE_URL = "http://localhost:8080"

def main():
    print("Starting verification...")
    
    # 1. Register Company
    company_email = f"verify_company_{int(time.time())}@example.com"
    company_data = {
        "name": "Verify Company",
        "email": company_email,
        "password": "Password123!",
        "isCompany": True
    }
    
    print(f"Registering company: {company_email}")
    res = requests.post(f"{BASE_URL}/auth/register/company", json=company_data)
    if res.status_code != 200:
        print(f"Registration failed: {res.text}")
        sys.exit(1)

    # 2. Login
    login_data = {
        "email": company_email,
        "password": "Password123!"
    }
    res = requests.post(f"{BASE_URL}/auth/login", json=login_data)
    if res.status_code != 200:
        print(f"Login failed: {res.text}")
        sys.exit(1)
    
    token = res.json().get("token")
    headers = {"Authorization": f"Bearer {token}"}
    print("Logged in.")

    # 3. Create Job
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
        sys.exit(1)
    
    job_id = res.json().get("_id")
    print(f"Job created: {job_id}")

    # 4. Update Job (Correct)
    update_data = {
        "salaryRange": {
            "min": 60000,
            "max": 90000
        }
    }
    
    print("\nUpdating job (Correct)...")
    res = requests.put(f"{BASE_URL}/jobs/{job_id}", json=update_data, headers=headers)
    if res.status_code != 200:
        print(f"Update job failed: {res.text}")
        sys.exit(1)
    print("Update successful.")

    # 5. Update Job (Incorrect - Partial)
    # This should now FAIL or throw an error because we enforce both min and max
    partial_update_data = {
        "salaryRange": {
            "min": 70000
        }
    }
    print("\nUpdating job (Incorrect - Partial)...")
    res = requests.put(f"{BASE_URL}/jobs/{job_id}", json=partial_update_data, headers=headers)
    print(f"Status: {res.status_code}")
    print(f"Response: {res.text}")
    
    if res.status_code == 400 and "salaryRange must contain min and max" in res.text:
        print("✅ SUCCESS: Partial update correctly rejected.")
    else:
        print("❌ FAILURE: Partial update was not rejected as expected.")
        sys.exit(1)

    # 6. Verify Data via GET
    print("\nVerifying data via GET...")
    res = requests.get(f"{BASE_URL}/jobs/{job_id}")
    if res.status_code != 200:
        print(f"Get job failed: {res.text}")
        sys.exit(1)
    
    job = res.json()
    sr = job.get("salaryRange")
    print(f"SalaryRange: {sr}")
    
    if sr["min"] == 60000 and sr["max"] == 90000:
        print("✅ SUCCESS: Data verified.")
    else:
        print("❌ FAILURE: Data mismatch.")
        sys.exit(1)

if __name__ == "__main__":
    main()
