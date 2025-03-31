import asyncio
import websockets
import wave
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QTextEdit, QLabel
from PyQt5.QtCore import QThread, pyqtSignal, QTimer
import os
import torch.nn as nn
import librosa
import numpy as np
import asyncio
import websockets
import wave
import paho.mqtt.client as mqtt
import noisereduce as nr
import torch
import soundfile as sf

broker = "mqtt-dashboard.com"
port = 8883
topic = "quang/home"
username = "hivemq.webclient.1731080373406"
password = "60W.>lvV@B9fGmzUA*4j"

# Khởi tạo MQTT client
client = mqtt.Client()
client.username_pw_set(username, password)
client.tls_set()
client.connect(broker, port, 60)
client.loop_start()

model_path = "C:/Users/ACER/Documents/A_PlatformIO/Test_MIC/py_model/model_full.pth"
input_file = "C:/Users/ACER/Documents/A_PlatformIO/Test_MIC/py_model/input_data/input_audio.wav"
output_dir = "C:/Users/ACER/Documents/A_PlatformIO/Test_MIC/py_model/preprocessed_files"
mfcc_file = "C:/Users/ACER/Documents/A_PlatformIO/Test_MIC/py_model/preprocessed_files/input_audio.npy"

class CNNModel(nn.Module):
    def __init__(self):
        super(CNNModel, self).__init__()
        self.conv1 = nn.Conv2d(1, 32, kernel_size=3, padding=1)  # Lớp convolution đầu tiên
        self.pool = nn.MaxPool2d(2, 2)  # Lớp max pooling
        self.conv2 = nn.Conv2d(32, 64, kernel_size=3, padding=1)  # Lớp convolution thứ hai
        self.fc1 = nn.Linear(64 * 3 * 15, 128)  # Lớp fully connected sau khi làm phẳng
        self.fc2 = nn.Linear(128, 4)  # Output 4 lớp phân loại

    def forward(self, x):
        x = x.unsqueeze(1)  # Thêm chiều kênh (C) cho dữ liệu 2D (sẽ thành [batch_size, 1, 13, 63])
        x = self.pool(torch.relu(self.conv1(x)))  # Convolution + Pooling
        x = self.pool(torch.relu(self.conv2(x)))  # Convolution + Pooling
        x = x.view(-1, 64 * 3 * 15)  # Làm phẳng dữ liệu với kích thước đúng
        x = torch.relu(self.fc1(x))  # Fully connected
        x = self.fc2(x)  # Output
        return x

def reduce_noise(input_path, output_path):
    try:
        audio_data, sample_rate = librosa.load(input_path, sr=None)
        reduced_noise = nr.reduce_noise(y=audio_data, sr=sample_rate)
        sf.write(output_path, reduced_noise, sample_rate)
    except Exception as e:
        print(f"Error processing {input_path}: {e}")

def trim_silence(input_path, output_path, top_db=20):
    try:
        audio_data, sample_rate = librosa.load(input_path, sr=None)
        trimmed_audio, _ = librosa.effects.trim(audio_data, top_db=top_db)
        sf.write(output_path, trimmed_audio, sample_rate)
    except Exception as e:
        print(f"Error processing {input_path}: {e}")

def resize_audio(input_path, output_path, target_length):
    try:
        audio_data, sample_rate = librosa.load(input_path, sr=None)
        current_length = len(audio_data)

        if current_length < target_length:
            padded_audio = np.pad(audio_data, (0, target_length - current_length), mode='constant')
        else:
            padded_audio = audio_data[:target_length]

        sf.write(output_path, padded_audio, sample_rate)
    except Exception as e:
        print(f"Error processing {input_path}: {e}")

def process_audio(input_path, output_directory, target_length=32000 , top_db=20):
    temp_file = os.path.join(output_directory, "temp_cleaned.wav")
    reduce_noise(input_path, temp_file)
    
    temp_trimmed_file = os.path.join(output_directory, "temp_trimmed.wav")
    trim_silence(temp_file, temp_trimmed_file, top_db)
    
    temp_resized_file = os.path.join(output_directory, "temp_resized.wav")
    resize_audio(temp_trimmed_file, temp_resized_file, target_length)
    
    try:
        y, sr = librosa.load(temp_resized_file, sr=None)
        mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)

        output_npy = os.path.join(output_directory, os.path.basename(input_path).replace('.wav', '.npy'))
        print(f"Saving MFCC to: {output_npy}")
        np.save(output_npy, mfccs)
        print(f"MFCC features saved at: {output_npy}")
        os.remove(temp_file)
        os.remove(temp_trimmed_file)
        os.remove(temp_resized_file)
    except Exception as e:
        print(f"Error processing {input_path}: {e}")
#process_audio(input_file, output_dir)

def predict_from_mfcc(mfcc_file, model_path):
    try:
        model = torch.load(model_path)
        model.eval()
        
        mfcc_data = np.load(mfcc_file)
        
        if mfcc_data.shape != (13, 63):
            print(f"MFCC features of {mfcc_file} are not in the desired shape.")
        else:
            mfcc_tensor = torch.tensor(mfcc_data, dtype=torch.float32).unsqueeze(0)
            device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
            mfcc_tensor = mfcc_tensor.to(device)
            model = model.to(device)

            with torch.no_grad():
                output = model(mfcc_tensor)
            
            predicted_class = torch.argmax(output, dim=1)
            print(f"Predicted class for {mfcc_file}: {predicted_class.item()}")
            return predicted_class.item()
            
    except Exception as e:
        print(f"Error processing {mfcc_file}: {e}")

class WebSocketClient(QThread):
    message_received = pyqtSignal(str)

    def __init__(self, uri):
        super().__init__()
        self.uri = uri
        self.running = False
        self.data = bytearray()
        self.loop = None
        self.websocket_task = None

    async def connect(self):
        try:
            async with websockets.connect(self.uri) as websocket:
                while self.running:
                    message = await websocket.recv()
                    self.data += message

                    # Hiển thị một số mẫu dữ liệu đầu tiên
                    num_samples_to_display = 10
                    sample_values = list(self.data[:num_samples_to_display * 2])
                    display_data = " ".join([str(int.from_bytes(sample_values[i:i+2], byteorder='little', signed=True))
                                             for i in range(0, len(sample_values), 2)])
                    self.message_received.emit("Received audio data: " + display_data)

        except websockets.exceptions.ConnectionClosedError:
            self.message_received.emit("WebSocket connection closed unexpectedly.")
        except asyncio.CancelledError:
            self.message_received.emit("Connection was cancelled.")
        except Exception as e:
            self.message_received.emit(f"Connection error: {str(e)}")

    def run(self):
        self.running = True
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.websocket_task = self.loop.create_task(self.connect())
        self.loop.run_forever()

        if self.websocket_task:
            self.websocket_task.cancel()
        self.loop.close()

    def stop(self):
        self.running = False
        if self.websocket_task and not self.websocket_task.done():
            self.websocket_task.cancel()
        if self.loop and self.loop.is_running():
            self.loop.call_soon_threadsafe(self.loop.stop)
        self.wait()

    def save_to_wav(self, filename):
        with wave.open(filename, 'wb') as wf:
            wf.setnchannels(1)
            wf.setsampwidth(2)
            wf.setframerate(16000)
            wf.writeframes(self.data)

class AudioRecorderApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('WebSocket Audio Recorder')
        self.client = WebSocketClient("ws://192.168.1.47:81")
        self.client.message_received.connect(self.update_message)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.stop_recording)
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout()

        # Khung hiển thị tin nhắn
        self.text_edit = QTextEdit(self)
        self.text_edit.setReadOnly(True)
        layout.addWidget(self.text_edit)

        # Nút bắt đầu ghi
        self.record_button = QPushButton("Bắt đầu ghi", self)
        self.record_button.clicked.connect(self.start_recording)
        layout.addWidget(self.record_button)

        # Nhãn hiển thị kết quả dự đoán
        self.predict_label = QLabel("Kết quả dự đoán: Chưa có", self)
        layout.addWidget(self.predict_label)

        self.setLayout(layout)

    def update_message(self, message):
        self.text_edit.append(message)

    def start_recording(self):
        self.client.data = bytearray()
        self.client.start()
        self.record_button.setEnabled(False)
        self.text_edit.append("Recording started...")
        self.timer.start(2000)

    def stop_recording(self):
        self.client.stop()
        self.record_button.setEnabled(True)
        self.text_edit.append("Recording stopped.")
        self.timer.stop()
        self.save_audio()  # Tự động gọi hàm lưu sau khi dừng ghi

    def save_audio(self):
        self.client.save_to_wav(input_file)
        self.text_edit.append("Audio saved to " + input_file)
        process_audio(input_file, output_dir)
        self.text_edit.append("Audio processed and MFCC features saved.")
        prediction = predict_from_mfcc(mfcc_file, model_path)

        # Cập nhật nhãn với kết quả dự đoán
        self.predict_label.setText(f"Kết quả dự đoán: {prediction}")
        if prediction is not None:
            client.publish(topic, str(prediction))
            print(f"Prediction sent to MQTT topic: {topic}, value: {prediction}")
        else:
            print("Prediction not sent. No valid action detected.")

    def closeEvent(self, event):
        self.client.stop()
        event.accept()

if __name__ == '__main__':
    app = QApplication([])
    window = AudioRecorderApp()
    window.show()
    app.exec_()