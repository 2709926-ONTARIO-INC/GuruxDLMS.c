from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QComboBox, QMainWindow
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QPushButton
from ServerPage import ServerPage
from utils import createLabel, open_next_page
import sys
class DetailsPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Details Page')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # Container widget
        container = QWidget(self)
        self.setCentralWidget(container)

        # Layouts
        layout = QVBoxLayout(container)
        layout.setAlignment(Qt.AlignCenter)

        # Title label
        title_label = createLabel("Please enter the following details", color =  "darkblue")
        layout.addWidget(title_label)
        layout.addSpacing(30)

        items = ["Host IP", "Standard ", "Country "]

        for item in items:
            hbox_layout = QHBoxLayout()
            label = createLabel(item, 12, alignment=False)
            input = QComboBox()
            input.setStyleSheet("background-color: white;")
            input.setFont(QFont("Arial", 12))
            input.setMaximumWidth(300)
            hbox_layout.addWidget(label)
            hbox_layout.addWidget(input)
            layout.addLayout(hbox_layout)
            layout.addSpacing(20)
            
         # Submit button
        submit_button = QPushButton("Submit and next",self)
        next_page = ServerPage()
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
    details_page = DetailsPage()
    details_page.show()
    sys.exit(app.exec_())