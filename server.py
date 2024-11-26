from flask import Flask, request, jsonify
from dotenv import load_dotenv
import os

load_dotenv(verbose=True)

LOCALHOST = os.getenv("IP4_ADDRESS")
PORT = os.getenv("PORT")

app = Flask(__name__)

@app.route("/api/store_data", methods=["POST"])
def store_data():
    try:
        data = request.json
        print(data)
        return jsonify({
            "status": True,
            "message": "Data stored successfully"
        }), 201
    except Exception as e:
        return jsonify({
                "status": False,
                "message": str(e)
            }), 500

if __name__ == "__main__":
    app.run(debug=True, port=int(PORT), host=LOCALHOST)