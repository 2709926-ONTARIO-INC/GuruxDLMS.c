from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QMainWindow, QHBoxLayout
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, createButton
import os

class LogoPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Logo Page")
        self.setFixedSize(1550, 725)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # Container widget
        container = QWidget(self)
        self.setCentralWidget(container)

        # Layout
        layout = QVBoxLayout(container)
        layout.setAlignment(Qt.AlignCenter)

        # Image
        image_label = QLabel(self)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        image_path = os.path.join(script_dir, "assets", "images", "logo.png")
        logo_pixmap = QPixmap(image_path)
        image_label.setPixmap(logo_pixmap)
        image_label.setAlignment(Qt.AlignCenter)

        # Title label
        title_label = createLabel("KIGG Smart Meter Simulator", 18, "darkblue")

        # Version label
        version_label = createLabel("v1.02", 10)

        # Next button
        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        next_button = createButton("Next")
        next_button.clicked.connect(lambda: openNextPage(self))
        button_layout.addWidget(next_button)

        # Adding widgets to the layout
        layout.addWidget(image_label)
        layout.addSpacing(20)
        layout.addWidget(title_label)
        layout.addSpacing(20)
        layout.addWidget(version_label)
        layout.addSpacing(20)
        layout.addLayout(button_layout)

        def openNextPage(self):
            from loginPage import LoginPage
            next_page = LoginPage()
            open_next_page(self, next_page)

if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    logo_page = LogoPage()
    logo_page.show()
    sys.exit(app.exec_())
