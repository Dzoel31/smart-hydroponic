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

        # dict_data = {
        #     "temp": data["temp"],
        #     "water_flow": data["water_flow"],
        #     "distance": data["distance"],
        #     "moisture": data["moisture"],
        #     "pump_status": data["pump_status"],
        #     "motor_status": data["motor_status"]
        # }
        dict_data = {
            "moisture": data["moisture"],
            "pump_status": data["pump_status"],
            "flow_rate": data["flow_rate"],
            "total_litres": data["total_litres"],
            "distance": data["distance"]
        }

        df = pd.DataFrame(dict_data, index=[0])
        
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
    
@app.route("/api/store_data_nutrition", methods=["POST"])
def store_data_nutrition():
    try:
        data = request.json

        dict_data = {
            "name": data["name"],
            "value": data["value"]
        }

        df = pd.DataFrame(dict_data, index=[0])

        if not os.path.exists("data_nutrition.csv"):
            df.to_csv("data_nutrition.csv", header=list(df.keys()), index=False)
        else:
            df.to_csv("data_nutrition.csv", mode="a", header=False, index=False)

        return jsonify({"status": True, "message": "Data stored successfully"}), 201
    except Exception as e:
        return jsonify({"status": False, "message": str(e)}), 500

# buat pompa untuk mengambil data sensor soil
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


@app.route("/api/get_latest_data", methods=["GET"])
def get_latest_data():
    try:
        data = pd.read_csv("data.csv")

        latest_data = data.iloc[-1].to_dict()

        return jsonify({
            "status": True,
            "data": latest_data
        }), 200
    except Exception as e:
        return jsonify({
            "status": False,
            "message": str(e)
        }), 500
    
@app.route("/api/get_data_nutrition", methods=["GET"])
def get_data_nutrition():
    try:
        data = pd.read_csv("data_nutrition.csv").to_dict(orient="records")

        return jsonify({"status": True, "data": data}), 200
    except Exception as e:
        return jsonify({"status": False, "message": str(e)}), 500

@app.route("/api/delete_data", methods=["DELETE"])
def delete_data():
    try:
        os.remove("data.csv")
        return jsonify({"status": True, "message": "Data deleted successfully"}), 200
    except Exception as e:
        return jsonify({"status": False, "message": str(e)}), 500
    
@app.route("/api/delete_data_nutrition", methods=["DELETE"])
def delete_data_nutrition():
    try:
        os.remove("data_nutrition.csv")
        return jsonify({"status": True, "message": "Data deleted successfully"}), 200
    except Exception as e:
        return jsonify({"status": False, "message": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True, port=int(PORT), host=LOCALHOST)