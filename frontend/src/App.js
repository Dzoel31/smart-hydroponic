import { useEffect, useState } from "react";
require('dotenv').config();

function SensorDashboard() {
    const [sensorData, setSensorData] = useState(null);

    useEffect(() => {
        const ws = new WebSocket(`ws://localhost:${process.env.PORT_WS}`);

        ws.onopen = () => {
            console.log('Connected to server');
        };

        ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            setSensorData(data);
        };

        return () => {
            ws.close();
        };
    }, []);

    return (
        <div>
            <h1>Sensor Dashboard</h1>
            {sensorData ? (
                <div>
                    <p>Temperature: {sensorData.temperature}</p>
                    <p>Humidity: {sensorData.humidity}</p>
                </div>
            ) : (
                <p>Loading...</p>
            )}
        </div>
    );
}

export default SensorDashboard;