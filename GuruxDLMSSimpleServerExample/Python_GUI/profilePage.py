from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QTableWidget, QHeaderView, QPushButton, QComboBox
from PyQt5.QtGui import QPixmap, QFont
from PyQt5.QtCore import Qt
from utils import createLabel, open_previous_page
import os
import json

class ClickableContainer(QPushButton):
    def __init__(self, image_path, label_text, parent_page):
        super().__init__()
        self.parent_page = parent_page
        self.table = parent_page.table
        self.createClickableContainer(image_path, label_text)

    def createClickableContainer(self, image_path, label_text):
        # Container widget
        container_layout = QVBoxLayout(self)
        container_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # Image
        image_label = QLabel()
        pixmap = QPixmap(image_path)
        pixmap = pixmap.scaled(150, 150, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        
        image_label.setPixmap(pixmap)
        image_label.setFixedSize(150, 150)
        image_label.setAlignment(Qt.AlignCenter)

        # Label
        text_label = createLabel(label_text, 12)

        container_layout.addWidget(image_label)
        container_layout.addWidget(text_label)

        self.setMinimumSize(175,210)
        self.clicked.connect(lambda: self.addTable())
        self.setStyleSheet(".QWidget{ border: 1px solid black; }")

    def addTable(self):
        table = ProfileTable()
        header_labels = ["Nameplate", ["Meter Serial Number", "Manufacturer Name", "Firmware Version For Meter", ]]
        if (self.parent_page.table):
            self.parent_page.main_layout.replaceWidget(self.table, table)
        else:
            self.parent_page.table = table
            self.table = table
            self.parent_page.main_layout.addWidget(table)

class ProfileTable(QWidget):
    def __init__(self):
        super().__init__()
        self.table = None
        self.createTable()

    def createTable(self):
        table_layout = QVBoxLayout()
        table_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        table = QTableWidget()
        table.setHorizontalHeaderLabels([])
        table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        table.setEditTriggers(QTableWidget.NoEditTriggers)
        table_layout.addWidget(table)
        self.setLayout(table_layout)

class ProfilePage(QWidget):
    def __init__(self):
        super().__init__()
        self.table = None
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Load Profile Page')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")

        # Main layout
        self.main_layout = QVBoxLayout(self)
        self.main_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # Title
        title_label = createLabel("Select Profile")
        self.main_layout.addWidget(title_label)

        # Container layout
        container_layout = QHBoxLayout()
        container_layout.setAlignment(Qt.AlignCenter)

        # List of images and labels
        script_dir = os.path.dirname(os.path.abspath(__file__))
        images_and_labels = [
            (os.path.join(script_dir, "assets", "images", "instant.png"), "Name Plate\nProfile"),
            (os.path.join(script_dir, "assets", "images", "mins15.png"), "Block Load\nProfile"),
            (os.path.join(script_dir, "assets", "images", "hrs24.png"), "Daily Load\nProfile"),
            (os.path.join(script_dir, "assets", "images", "days30.png"), "Billing\nProfile"),
        ]

        # Create containers with unique images and labels
        for image_path, label_text in images_and_labels:
            container = ClickableContainer(image_path, label_text, self)
            container_layout.addWidget(container)

        selection = ["Subtest", "Meter Type", "Meter No."]
        f =  open("simulation.json")
        meter_data = json.load(f)
        for item in selection:
            input_container = QWidget()
            input_container.setMaximumSize(200, 100)
            select_layout = QVBoxLayout()
            label = createLabel(item, 12)
            input = QComboBox()
            input.setStyleSheet("background-color: white;")
            if item == "Meter Type":
                input.addItems(["Single Phase", "Three Phase"])
            elif item == "Subtest":
                for i in meter_data:
                    input.addItem(i[0])
            select_layout.addWidget(label)
            select_layout.addWidget(input)
            input_container.setLayout(select_layout)
            container_layout.addWidget(input_container)

        self.main_layout.addLayout(container_layout)

        back_btn = QPushButton("Back", self)
        back_btn.clicked.connect(lambda: self.openPrevPage())
        back_btn.setFont(QFont("Arial", 12))
        back_btn.setMinimumWidth(100)
        back_btn.setStyleSheet(
            "background-color: white; border: 1px solid black; border-radius: 5px;padding:8px"
        )

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(back_btn)
        self.main_layout.addLayout(button_layout) 

    def openPrevPage(self):
        from eventsSimulation import EventSimulationApp
        prev_page = EventSimulationApp()
        open_previous_page(self, prev_page)

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    page = ProfilePage()
    page.show()
    sys.exit(app.exec_())
