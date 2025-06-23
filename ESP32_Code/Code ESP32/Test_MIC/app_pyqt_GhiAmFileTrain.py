import asyncio
import websockets
import wave
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QTextEdit
from PyQt5.QtCore import QThread, pyqtSignal
import os
from PyQt5.QtCore import QTimer

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
        self.loop.run_forever()  # Chạy vòng lặp mãi mãi thay vì run_until_complete

        # Dừng vòng lặp khi kết thúc
        if self.websocket_task:
            self.websocket_task.cancel()
        self.loop.close()

    def stop(self):
        self.running = False
        if self.websocket_task and not self.websocket_task.done():
            self.websocket_task.cancel()  # Hủy task connect đang chạy
        if self.loop and self.loop.is_running():
            self.loop.call_soon_threadsafe(self.loop.stop)  # Dừng vòng lặp an toàn
        self.wait()  # Đợi cho thread kết thúc

    def save_to_wav(self, filename):
        # Lưu dữ liệu âm thanh thành file WAV
        with wave.open(filename, 'wb') as wf:
            wf.setnchannels(1)
            wf.setsampwidth(2)
            wf.setframerate(16000)
            wf.writeframes(self.data)

from PyQt5.QtCore import QTimer  # Thêm QTimer

class AudioRecorderApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('WebSocket Audio Recorder')
        self.client = WebSocketClient("ws://192.168.1.47:81")  # Địa chỉ WebSocket của ESP32
        self.client.message_received.connect(self.update_message)
        self.timer = QTimer(self)  # Tạo một timer
        self.timer.timeout.connect(self.stop_recording)  # Kết nối timer với phương thức stop_recording
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout()
        self.text_edit = QTextEdit(self)
        self.text_edit.setReadOnly(True)
        layout.addWidget(self.text_edit)

        self.record_button = QPushButton("Bắt đầu ghi", self)
        self.record_button.clicked.connect(self.start_recording)
        layout.addWidget(self.record_button)

        self.stop_button = QPushButton("Dừng ghi", self)
        self.stop_button.setEnabled(False)
        self.stop_button.clicked.connect(self.stop_recording)
        layout.addWidget(self.stop_button)

        self.save_button = QPushButton("Lưu file WAV", self)
        self.save_button.clicked.connect(self.save_audio)
        layout.addWidget(self.save_button)

        self.setLayout(layout)

    def update_message(self, message):
        self.text_edit.append(message)

    def start_recording(self):
        self.client.data = bytearray()  # Xóa dữ liệu cũ trước khi bắt đầu ghi
        self.client.start()  # Bắt đầu nhận dữ liệu
        self.record_button.setEnabled(False)
        self.stop_button.setEnabled(True)
        self.text_edit.append("Recording started...")

        # Bắt đầu đếm ngược 2 giây
        self.timer.start(2000)

    def stop_recording(self):
        self.client.stop()  # Dừng nhận dữ liệu
        self.record_button.setEnabled(True)
        self.stop_button.setEnabled(False)
        self.text_edit.append("Recording stopped.")
        self.timer.stop()  # Dừng timer

    def save_audio(self):
        # Thư mục để lưu file WAV
        folder_path = "C:/Users/ACER/Documents/A_PlatformIO/Test_MIC/audio_recorded_samples"
        os.makedirs(folder_path, exist_ok=True)  # Tạo thư mục nếu chưa tồn tại

        # Tìm tên file chưa tồn tại
        file_index = 14
        while True:
            filename = os.path.join(folder_path, f"output160_{file_index}.wav")
            if not os.path.exists(filename):
                break
            file_index += 1

        # Lưu file với tên mới
        self.client.save_to_wav(filename)
        self.text_edit.append(f"Audio saved as '{filename}'")

    def closeEvent(self, event):
        # Đảm bảo client dừng khi đóng cửa sổ ứng dụng
        self.client.stop()
        event.accept()


if __name__ == '__main__':
    app = QApplication([])
    window = AudioRecorderApp()
    window.show()
    app.exec_()
