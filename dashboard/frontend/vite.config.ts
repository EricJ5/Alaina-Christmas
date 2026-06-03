import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
	plugins: [react()],
	base: './',
	server: {
		proxy: {
			'/api': {
				target: 'http://localhost:8080', // Your Go server URL
				changeOrigin: true,
			},      // Forwards WebSocket connections
			'/ws': {
				target: 'ws://localhost:8080',
				ws: true, // Crucial for WebSocket support
				changeOrigin: true,
			},
		},
	},
})
