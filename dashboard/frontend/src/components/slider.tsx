import { useState, useEffect, useRef } from "react";
import Box from '@mui/material/Box';
import Slider from '@mui/material/Slider';

interface MB_SliderProps {
	socket: WebSocket | null;
	getterEvent: string;
	setterEvent: string;
	minValue?: number;
	maxValue?: number;
	step?: number;

}
interface WebsocketMessage {
	eventName: string;
	data: Number | String;

}

export default function MB_Slider({ socket, getterEvent, setterEvent, minValue, maxValue, step }: MB_SliderProps) {
	const [value, setValue] = useState<number>(0);
	const lastRan = useRef<number>(Date.now())
	const isDragging = useRef<boolean>(false);
	const throttleLimit = 50;
	const latestValue = useRef<number>(value)
	const throttleTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);
	latestValue.current = value;
	useEffect(() => {
		const sendInitial = () => {
			if (socket && socket.readyState == WebSocket.OPEN) {
				socket?.send(JSON.stringify({
					eventName: getterEvent,
					data: null
				}))
			}
		}

		if (socket) {
			if (socket && socket.readyState == WebSocket.OPEN) {
				sendInitial()
			} else if (socket.readyState == WebSocket.CONNECTING) {
				socket.addEventListener("open", sendInitial, { once: true })
			}

		}

		return () => {
			socket?.removeEventListener("open", sendInitial)
		}

	}, [socket, getterEvent])
	useEffect(() => {
		const listenForData = (message: MessageEvent): void => {
			try {

				const messageData = JSON.parse(message.data) as WebsocketMessage

				if (messageData.eventName !== setterEvent || messageData.data === null) {
					return
				}

				if (!isDragging.current && messageData.data !== latestValue.current) {
					setValue(messageData.data as number);
				}
			} catch (error) {
				console.error("failed to parse message frame: ", error)
			}
		};
		socket?.addEventListener("message", listenForData);
		return () => {
			socket?.removeEventListener("message", listenForData);
		};
	}, [socket, setterEvent])
	const sendData = () => {
		if (socket && socket.readyState == WebSocket.OPEN) {
			socket.send(JSON.stringify({
				eventName: setterEvent,
				data: latestValue.current
			}))
			lastRan.current = Date.now();
		}

	}
	const handleChange = (_event: Event, newValue: number | number[]) => {
		setValue(newValue as number);

		if (throttleTimeoutRef.current) {
			clearTimeout(throttleTimeoutRef.current);
		}

		const timeSinceLastRun = Date.now() - lastRan.current;

		if (timeSinceLastRun >= throttleLimit) {
			sendData()
		} else {
			throttleTimeoutRef.current = setTimeout(sendData, throttleLimit - timeSinceLastRun);
		}


	}

	useEffect(() => {
		return () => {
			if (throttleTimeoutRef.current) clearTimeout(throttleTimeoutRef.current)
		}

	}, [])
	return (
		<Box sx={{ width: 300 }}>
			<Slider
				size="small"
				aria-label="Small"
				value={value}
				max={maxValue}
				min={minValue}
				step={step}
				onMouseDown={() => { isDragging.current = true; }}
				onMouseUp={() => { isDragging.current = false; }}
				onTouchStart={() => { isDragging.current = true; }}
				onTouchEnd={() => { isDragging.current = false; }}
				valueLabelDisplay="auto"
				onChange={handleChange}
			/>
		</Box>
	);
}
