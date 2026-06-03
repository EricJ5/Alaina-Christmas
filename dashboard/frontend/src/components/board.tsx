import MB_Slider from "./slider"
import { useEffect, useState } from "react"


export default function MB_Board() {
	const [socket, setSocket] = useState<WebSocket | null>(null);



	useEffect(() => {
		const ws = new WebSocket(`${window.location.protocol === 'https:' ? 'wss' : 'ws'}://${window.location.host}/ws`);
		setSocket(ws)
		return () => ws.close();

	}, [])
	return (
		<div style={{ padding: '20px' }}>
			<h1>Controls</h1>
			<h3>Volume Control</h3>
			<MB_Slider
				socket={socket}
				getterEvent="getVolume"
				setterEvent="setVolume"
			/>
			<h3>Minimum voltage</h3>
			<p>
				the higher it is the faster you have to spin to create music.And the slower it will go in speed mode
			</p>
			<MB_Slider
				socket={socket}
				getterEvent="getVoltage"
				setterEvent="setVoltage"
				minValue={0}
				maxValue={3.3}
				step={0.001}
			/>

		</div>
	)
}
