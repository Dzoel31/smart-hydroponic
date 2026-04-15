import requests
import dotenv
import os

dotenv.load_dotenv()

BASE_URL = f"http://{os.getenv('HOST')}:{os.getenv('PORT')}/api/smart-hydroponic/v1"


def register():
    print("\n=== Register ===")
    username = input("Username: ")
    password = input("Password: ")

    data = {"username": username, "password": password}
    res = requests.post(f"{BASE_URL}/register", json=data)

    try:
        print("[REGISTER]", res.status_code, res.json())
    except Exception:
        print("Response:", res.text)


def login():
    print("\n=== Login ===")
    username = input("Username: ")
    password = input("Password: ")

    data = {"username": username, "password": password}
    res = requests.post(f"{BASE_URL}/login", json=data)

    try:
        response = res.json()
        print("[LOGIN]", res.status_code, response)
        if "token" in response:
            return response["token"]
    except Exception:
        print("Response:", res.text)

    return None


def access_protected(token):
    print("\n=== Access Protected ===")
    headers = {"Authorization": f"Bearer {token}"}
    res = requests.get(f"{BASE_URL}/protected", headers=headers)

    try:
        print("[PROTECTED]", res.status_code, res.json())
    except Exception:
        print("Response:", res.text)


def main():
    token = None
    while True:
        print("\n=== Menu ===")
        print("1. Register")
        print("2. Login")
        print("3. Access Protected Route")
        print("4. Exit")

        choice = input("Choose an option: ")

        if choice == "1":
            register()
        elif choice == "2":
            token = login()
        elif choice == "3":
            if token:
                access_protected(token)
            else:
                print("You need to login first.")
        elif choice == "4":
            print("Exiting...")
            break
        else:
            print("Invalid choice, please try again.")


if __name__ == "__main__":
    main()
