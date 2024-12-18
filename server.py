from flask import Flask, request, jsonify
from dotenv import load_dotenv
import os
import sys

import pandas as pd

load_dotenv(verbose=True)

LOCALHOST = os.getenv("IP4_ADDRESS")
PORT = os.getenv("PORT")

app = Flask(__name__)

@app.route("/api/store_data", methods=["POST"])
def store_data():
    try:
        data = request.json

        df = pd.DataFrame(data, index=[0])
        
        if not os.path.exists("data.csv"):
            df.to_csv("data.csv", header=list(df.keys()), index=False)
        else:
            df.to_csv("data.csv", mode="a", header=False, index=False)

        return jsonify({
            "status": True,
            "message": "Data stored successfully"
        }), 201

    except Exception as e:
        return jsonify({
                "status": False,
                "message": str(e)
            }), 500

@app.route("/api/get_data", methods=["GET"])
def get_data():
    try:
        data = pd.read_csv("data.csv").to_dict(orient="records")

        return jsonify({
            "status": True,
            "data": data
        }), 200
    except Exception as e:
        return jsonify({
            "status": False,
            "message": str(e)
        }), 500

if __name__ == "__main__":
    app.run(debug=True, port=int(PORT), host=LOCALHOST)