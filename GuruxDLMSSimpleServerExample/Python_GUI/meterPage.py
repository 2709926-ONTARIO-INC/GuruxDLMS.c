import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLineEdit, QVBoxLayout, QWidget, QHBoxLayout, QFrame
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QPushButton
from meterConfig import MeterConfig
from utils import createLabel, open_next_page

class MeterPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Meter Page")
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        container = QWidget(self)
        self.setCentralWidget(container)

        layout = QVBoxLayout(container)
        layout.setAlignment(Qt.AlignCenter)

        # Title Label
        title = createLabel("Please enter the number of meters of each type", color = "blue")
        layout.addWidget(title)
        layout.addSpacing(30)

        meters = ["Single Phase", "Three Phase (WC)", "Three Phase (LTCT)", "Three Phase (HTCT)"]

        for meter in meters:
            frame = QFrame()
            frame.setMaximumWidth(650)
            hbox_layout = QHBoxLayout(frame)
            label = createLabel(meter, 12, alignment=False)
            input = QLineEdit()
            input.setStyleSheet("background-color: white;")
            input.setFont(QFont("Arial", 12))
            input.setMaximumWidth(300)
            hbox_layout.addWidget(label)
            hbox_layout.addWidget(input)
            layout.addWidget(frame)
            layout.addSpacing(10)

        # Submit button
        submit_button = QPushButton("Submit and next",self)
        next_page = MeterConfig()
        submit_button.clicked.connect(lambda: open_next_page(self, next_page))
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(submit_button)
        layout.addLayout(button_layout)

        # Ensure container resizes with the window
        container.setLayout(layout)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    meter_page = MeterPage()
    meter_page.show()
    sys.exit(app.exec_())
