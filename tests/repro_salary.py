import urllib.request
import json
import sys

BASE_URL = "http://localhost:8080"

def request(method, endpoint, data=None, token=None):
    url = BASE_URL + endpoint
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    
    json_data = json.dumps(data).encode("utf-8") if data else None
    req = urllib.request.Request(url, data=json_data, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as response:
            res_body = response.read().decode("utf-8")
            if res_body:
                try:
                    return json.loads(res_body)
                except:
                    return res_body
            return None
    except Exception as e:
        print(f"Error: {e}")
        return None

def main():
    # 1. Login Company
    company_auth = request("POST", "/auth/login", {"email": "company10000@test.com", "password": "password123"})
    if not company_auth:
        # Register if not exists (lazy way, assuming test_full_coverage ran)
        print("Login failed, trying to register...")
        request("POST", "/auth/register/company", {"email": "company10000@test.com", "password": "password123"})
        company_auth = request("POST", "/auth/login", {"email": "company10000@test.com", "password": "password123"})
    
    if not company_auth:
        print("Could not login company")
        sys.exit(1)

    token = company_auth["token"]
    
    # 2. Create Job
    job_data = {
        "title": "Salary Test Job",
        "description": "Desc",
        "requiredLanguages": ["C++"],
        "grade": "Junior",
        "skills": ["None"],
        "category": "Test",
        "salaryRange": {"min": 100, "max": 200},
        "location": "Remote",
        "workType": ["Full-time"]
    }
    job_res = request("POST", "/jobs", job_data, token=token)
    job_id = job_res["_id"]
    print(f"Created Job: {job_id}")

    # 3. Verify Initial Salary Range (should be object)
    job = request("GET", f"/jobs/{job_id}")
    print(f"Initial SalaryRange Type: {type(job['salaryRange'])}")
    print(f"Initial SalaryRange Value: {job['salaryRange']}")

    # 4. Update Job with Stringified SalaryRange (simulating the issue user might be doing, or just normal update)
    # Case A: Update with Object
    update_data = {"salaryRange": {"min": 300, "max": 400}}
    request("PUT", f"/jobs/{job_id}", update_data, token=token)
    
    job_after = request("GET", f"/jobs/{job_id}")
    print(f"After Object Update - Type: {type(job_after['salaryRange'])}")
    print(f"After Object Update - Value: {job_after['salaryRange']}")

    if isinstance(job_after['salaryRange'], str):
        print("FAIL: salaryRange is a string!")
    else:
        print("PASS: salaryRange is an object")

    # Case B: Update with String (if supported)
    update_data_str = {"salaryRange": "{\"min\": 500, \"max\": 600}"}
    request("PUT", f"/jobs/{job_id}", update_data_str, token=token)
    
    job_after_str = request("GET", f"/jobs/{job_id}")
    print(f"After String Update - Type: {type(job_after_str['salaryRange'])}")
    print(f"After String Update - Value: {job_after_str['salaryRange']}")

    if isinstance(job_after_str['salaryRange'], str):
        print("FAIL: salaryRange is a string!")
    else:
        print("PASS: salaryRange is an object")

    # Cleanup
    request("DELETE", f"/jobs/{job_id}", token=token)

if __name__ == "__main__":
    main()
