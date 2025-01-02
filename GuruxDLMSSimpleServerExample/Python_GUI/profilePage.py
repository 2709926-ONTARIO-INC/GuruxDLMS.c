from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QTableWidget, QHeaderView, QPushButton
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import Qt
from utils import createLabel
import os

class ProfileTable(QWidget):
    def __init__(self):
        super().__init__()
        self.createTable()

    def createTable(self):
        table_layout = QVBoxLayout()
        table_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        table = QTableWidget(0,3)
        table.setHorizontalHeaderLabels(["Column 1", "Column 2", "Column 3"])
        table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        table_layout.addWidget(table)
        self.setLayout(table_layout)

class ProfilePage(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.table = None

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
            container = self.createClickableContainer(image_path, label_text)
            container_layout.addWidget(container)

        self.main_layout.addLayout(container_layout)

    def createClickableContainer(self, image_path, label_text):
        # Container widget
        container = QPushButton()
        container_layout = QVBoxLayout(container)
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

        container.setMinimumSize(175,210)
        container.clicked.connect(lambda: self.addTable())
        container.setStyleSheet(".QWidget{ border: 1px solid black; }")

        return container
    
    def addTable(self):
        table = ProfileTable()
        if (self.table):
            self.main_layout.replaceWidget(self.table, table)
        else:
            self.table = table
            self.main_layout.addWidget(table)

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    page = ProfilePage()
    page.show()
    sys.exit(app.exec_())
