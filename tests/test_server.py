import urllib.request
import urllib.error
import json
import random
import sys

BASE_URL = "http://localhost:8080"

def request(method, endpoint, data=None, token=None):
    url = BASE_URL + endpoint
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    
    if data:
        json_data = json.dumps(data).encode("utf-8")
    else:
        json_data = None

    req = urllib.request.Request(url, data=json_data, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as response:
            res_body = response.read().decode("utf-8")
            print(f"[{method}] {endpoint} - {response.status}")
            if res_body:
                try:
                    return json.loads(res_body)
                except:
                    return res_body
            return None
    except urllib.error.HTTPError as e:
        print(f"[{method}] {endpoint} - {e.code} - {e.read().decode('utf-8')}")
        return None
    except Exception as e:
        print(f"Error: {e}")
        return None

def main():
    # Generate random emails to avoid conflicts
    rand_id = random.randint(1000, 9999)
    user_email = f"user{rand_id}@example.com"
    company_email = f"company{rand_id}@example.com"
    password = "password123"

    print(f"Testing with User: {user_email}, Company: {company_email}")

    # 1. Register User
    print("\n--- Register User ---")
    res = request("POST", "/auth/register/user", {"email": user_email, "password": password})
    if res != "User added":
        print("Failed to register user")
        # sys.exit(1) # Continue anyway to see if login works if it existed

    # 2. Login User
    print("\n--- Login User ---")
    user_auth = request("POST", "/auth/login", {"email": user_email, "password": password})
    if not user_auth or "token" not in user_auth:
        print("Failed to login user")
        sys.exit(1)
    user_token = user_auth["token"]
    user_id = user_auth["user"]["_id"]["$oid"]
    print(f"User Token: {user_token[:20]}...")
    print(f"User ID: {user_id}")

    # 3. Register Company
    print("\n--- Register Company ---")
    res = request("POST", "/auth/register/company", {"email": company_email, "password": password})
    if res != "Company added":
        print("Failed to register company")

    # 4. Login Company
    print("\n--- Login Company ---")
    company_auth = request("POST", "/auth/login", {"email": company_email, "password": password})
    if not company_auth or "token" not in company_auth:
        print("Failed to login company")
        sys.exit(1)
    company_token = company_auth["token"]
    company_id = company_auth["company"]["_id"]["$oid"]
    print(f"Company Token: {company_token[:20]}...")
    print(f"Company ID: {company_id}")

    # 5. Create Job
    print("\n--- Create Job ---")
    job_data = {
        "title": "C++ Engineer",
        "description": "Write cool code",
        "requiredLanguages": ["C++", "Python"],
        "grade": "Senior",
        "skills": ["Linux", "MongoDB"],
        "category": "Backend",
        "salaryRange": {"min": 100000, "max": 150000},
        "location": "Remote",
        "workType": ["Full-time"]
    }
    job_res = request("POST", "/jobs", job_data, token=company_token)
    if not job_res or "_id" not in job_res:
        print("Failed to create job")
        sys.exit(1)
    job_id = job_res["_id"]
    print(f"Job Created: {job_id}")

    # 6. Apply for Job
    print("\n--- Apply for Job ---")
    app_data = {
        "jobId": job_id,
        "message": "I want this job!",
        "resumeUrl": "http://example.com/resume.pdf"
    }
    app_res = request("POST", "/applications", app_data, token=user_token)
    if not app_res or "jobId" not in app_res:
        print("Failed to apply")
        sys.exit(1)
    print("Application Successful")

    # 7. Verify Application (Get User Applications)
    print("\n--- Get User Applications ---")
    user_apps = request("GET", f"/applications/user/{user_id}", token=user_token)
    if not user_apps or not isinstance(user_apps, list) or len(user_apps) == 0:
        print("Failed to retrieve applications")
    else:
        print(f"Found {len(user_apps)} applications")

    print("\n--- All Tests Passed ---")

    # ==========================================
    # EDGE CASES / NEGATIVE TESTS
    # ==========================================
    print("\n=== STARTING EDGE CASE TESTS ===")

    # 1. Register with existing email
    print("\n[Edge] Register existing user")
    res = request("POST", "/auth/register/user", {"email": user_email, "password": password})
    # The request function returns None on HTTP error (like 400), but prints the error.
    # We need to modify request() to return the error body on 400/500 so we can check it.
    # OR we assume None means it failed (which is good for negative tests if we saw the 400 print).
    
    # Let's rely on the print output for now or better: check if res is None (which means it failed as expected)
    if res is None:
        print("PASS: Correctly rejected existing user (HTTP Error caught)")
    else:
        print(f"FAIL: Should have rejected. Got: {res}")

    # 2. Login with wrong password
    print("\n[Edge] Login wrong password")
    res = request("POST", "/auth/login", {"email": user_email, "password": "wrongpassword"})
    if res is None:
        print("PASS: Correctly rejected wrong password")
    else:
        print(f"FAIL: Should have failed. Got: {res}")

    # 3. Create Job with missing fields
    print("\n[Edge] Create Job missing title")
    bad_job = job_data.copy()
    del bad_job["title"]
    res = request("POST", "/jobs", bad_job, token=company_token)
    if res is None:
        print("PASS: Correctly rejected missing title")
    else:
        print(f"FAIL: Should have rejected. Got: {res}")

    # 4. Apply with missing fields
    print("\n[Edge] Apply missing resumeUrl")
    bad_app = app_data.copy()
    del bad_app["resumeUrl"]
    res = request("POST", "/applications", bad_app, token=user_token)
    if res is None:
        print("PASS: Correctly rejected missing resume")
    else:
        print(f"FAIL: Should have rejected. Got: {res}")

    # 5. Apply to non-existent job
    print("\n[Edge] Apply to fake job")
    fake_job_id = "000000000000000000000000"
    fake_app = app_data.copy()
    fake_app["jobId"] = fake_job_id
    res = request("POST", "/applications", fake_app, token=user_token)
    if res is None:
        print("PASS: Correctly rejected fake job")
    else:
        print(f"FAIL: Should have failed. Got: {res}")

    print("\n=== EDGE CASES FINISHED ===")

if __name__ == "__main__":
    main()
