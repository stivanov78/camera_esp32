#!/usr/bin/env python3
"""
ESP32-CAM Video Stream Client

This Python script connects to an ESP32-CAM MJPEG stream and displays
the video using OpenCV. It supports both continuous streaming and single
image capture modes.

Requirements:
    - opencv-python (cv2)
    - numpy
    - requests

Install requirements:
    pip install opencv-python numpy requests

Usage:
    python receive_video_stream.py
    
Author: ESP32-CAM Project
License: MIT
"""

import cv2
import numpy as np
import requests
import time
import sys
from urllib.parse import urljoin

# Configuration
DEFAULT_IP = "192.168.1.100"
STREAM_ENDPOINT = "/stream"
CAPTURE_ENDPOINT = "/capture"

# JPEG markers for boundary detection
JPEG_START_MARKER = b'\xff\xd8'  # Start of Image (SOI)
JPEG_END_MARKER = b'\xff\xd9'    # End of Image (EOI)


class ESP32CamClient:
    """Client for receiving video stream from ESP32-CAM"""
    
    def __init__(self, ip_address):
        """
        Initialize the ESP32-CAM client
        
        Args:
            ip_address (str): IP address of the ESP32-CAM
        """
        self.ip_address = ip_address
        self.base_url = f"http://{ip_address}"
        self.stream_url = urljoin(self.base_url, STREAM_ENDPOINT)
        self.capture_url = urljoin(self.base_url, CAPTURE_ENDPOINT)
        
    def test_connection(self):
        """
        Test connection to ESP32-CAM
        
        Returns:
            bool: True if connection successful, False otherwise
        """
        try:
            print(f"Testing connection to {self.base_url}...")
            response = requests.get(self.base_url, timeout=5)
            if response.status_code == 200:
                print("✓ Connection successful!")
                return True
            else:
                print(f"✗ Connection failed with status code: {response.status_code}")
                return False
        except requests.exceptions.RequestException as e:
            print(f"✗ Connection failed: {e}")
            return False
    
    def capture_single_image(self):
        """
        Capture a single image from ESP32-CAM
        
        Returns:
            bool: True if capture successful, False otherwise
        """
        try:
            print(f"\nCapturing image from {self.capture_url}...")
            response = requests.get(self.capture_url, timeout=10)
            
            if response.status_code == 200:
                # Convert bytes to numpy array
                image_array = np.frombuffer(response.content, dtype=np.uint8)
                
                # Decode image
                image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
                
                if image is not None:
                    # Display the image
                    cv2.imshow('ESP32-CAM Capture', image)
                    
                    # Save the image
                    filename = 'esp32_capture.jpg'
                    cv2.imwrite(filename, image)
                    print(f"✓ Image captured and saved as '{filename}'")
                    print("Press any key to close the image window...")
                    cv2.waitKey(0)
                    cv2.destroyAllWindows()
                    return True
                else:
                    print("✗ Failed to decode image")
                    return False
            else:
                print(f"✗ Capture failed with status code: {response.status_code}")
                return False
                
        except requests.exceptions.RequestException as e:
            print(f"✗ Capture failed: {e}")
            return False
        except Exception as e:
            print(f"✗ Error: {e}")
            return False
    
    def stream_video(self):
        """
        Stream video from ESP32-CAM and display it
        
        Uses MJPEG stream parsing with JPEG boundary detection
        """
        try:
            print(f"\nConnecting to video stream: {self.stream_url}")
            print("Press 'q' to quit streaming\n")
            
            # Open stream with requests
            response = requests.get(self.stream_url, stream=True, timeout=10)
            
            if response.status_code != 200:
                print(f"✗ Failed to connect to stream. Status code: {response.status_code}")
                return False
            
            print("✓ Connected to stream!")
            print("Receiving video frames...\n")
            
            # Buffer for accumulating data
            buffer = b''
            frame_count = 0
            start_time = time.time()
            
            # Read stream in chunks
            for chunk in response.iter_content(chunk_size=1024):
                buffer += chunk
                
                # Look for JPEG boundaries
                while True:
                    # Find start of JPEG
                    start_idx = buffer.find(JPEG_START_MARKER)
                    if start_idx == -1:
                        break
                    
                    # Find end of JPEG
                    end_idx = buffer.find(JPEG_END_MARKER, start_idx)
                    if end_idx == -1:
                        break
                    
                    # Extract JPEG image (include end marker)
                    jpg_data = buffer[start_idx:end_idx + 2]
                    buffer = buffer[end_idx + 2:]
                    
                    # Decode JPEG
                    image_array = np.frombuffer(jpg_data, dtype=np.uint8)
                    frame = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
                    
                    if frame is not None:
                        # Calculate FPS
                        frame_count += 1
                        elapsed_time = time.time() - start_time
                        if elapsed_time > 0:
                            fps = frame_count / elapsed_time
                            
                            # Add FPS text to frame
                            cv2.putText(frame, f'FPS: {fps:.2f}', (10, 30),
                                      cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                            cv2.putText(frame, f'Frame: {frame_count}', (10, 60),
                                      cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                        
                        # Display frame
                        cv2.imshow('ESP32-CAM Video Stream', frame)
                        
                        # Check for quit key
                        if cv2.waitKey(1) & 0xFF == ord('q'):
                            print("\n✓ Streaming stopped by user")
                            break
                    
                    # Print status every 30 frames
                    if frame_count % 30 == 0:
                        print(f"Received {frame_count} frames (FPS: {fps:.2f})")
                
                # Check if user pressed 'q' to quit
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
            
            # Cleanup
            cv2.destroyAllWindows()
            
            # Print summary
            elapsed_time = time.time() - start_time
            if elapsed_time > 0:
                avg_fps = frame_count / elapsed_time
                print(f"\nStream Summary:")
                print(f"- Total frames: {frame_count}")
                print(f"- Duration: {elapsed_time:.2f} seconds")
                print(f"- Average FPS: {avg_fps:.2f}")
            
            return True
            
        except requests.exceptions.RequestException as e:
            print(f"✗ Stream error: {e}")
            cv2.destroyAllWindows()
            return False
        except KeyboardInterrupt:
            print("\n✓ Streaming stopped by user (Ctrl+C)")
            cv2.destroyAllWindows()
            return True
        except Exception as e:
            print(f"✗ Error: {e}")
            cv2.destroyAllWindows()
            return False


def print_banner():
    """Print application banner"""
    print("=" * 50)
    print("ESP32-CAM Video Stream Client")
    print("=" * 50)
    print()


def get_ip_address():
    """
    Get ESP32-CAM IP address from user
    
    Returns:
        str: IP address
    """
    ip = input(f"Enter ESP32-CAM IP address (default: {DEFAULT_IP}): ").strip()
    if not ip:
        ip = DEFAULT_IP
    return ip


def show_menu():
    """
    Display menu and get user choice
    
    Returns:
        str: User's menu choice
    """
    print("\nSelect mode:")
    print("1. Stream video (continuous)")
    print("2. Capture single image")
    print("3. Exit")
    print()
    choice = input("Enter your choice (1-3): ").strip()
    return choice


def main():
    """Main application entry point"""
    print_banner()
    
    # Get IP address
    ip_address = get_ip_address()
    
    # Create client
    client = ESP32CamClient(ip_address)
    
    # Test connection
    if not client.test_connection():
        print("\nFailed to connect to ESP32-CAM.")
        print("Please check:")
        print("1. ESP32-CAM is powered on")
        print("2. ESP32-CAM is connected to WiFi")
        print("3. IP address is correct")
        print("4. You are on the same network")
        sys.exit(1)
    
    # Main loop
    while True:
        choice = show_menu()
        
        if choice == '1':
            # Stream video
            client.stream_video()
        elif choice == '2':
            # Capture single image
            client.capture_single_image()
        elif choice == '3':
            # Exit
            print("\nGoodbye!")
            break
        else:
            print("\n✗ Invalid choice. Please enter 1, 2, or 3.")
    
    sys.exit(0)


if __name__ == "__main__":
    main()
