from PyQt5.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QTableWidget, QApplication, QHBoxLayout, QPushButton, QHeaderView, QTableWidgetItem
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from meterConfig import MeterConfig
from utils import createLabel, open_next_page
import sys

class ServerPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Server Page')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")

        # Main layout
        main_layout = QVBoxLayout(self)
        main_layout.setAlignment(Qt.AlignCenter)

        title_label = createLabel("Please enter the server details", 20, "darkblue")
        main_layout.addWidget(title_label)

        self.table = QTableWidget(self)
        self.table.setColumnCount(6)
        self.table.setHorizontalHeaderLabels(["No. of Meters", "Path to Binary file", "Path to Config file", "Starting Port No.", "Starting Instance no.", "Garbage Values"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
            "QAbstractItemView::indicator { width: 25px; height: 25px }"
        )
        main_layout.addWidget(self.table)

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        add_btn = QPushButton("Add Row", self)
        add_btn.clicked.connect(lambda: self.addRow())
        add_btn.setFont(QFont("Arial", 12))
        add_btn.setMinimumWidth(100)
        add_btn.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
        button_layout.addWidget(add_btn)

        # Submit button
        submit_button = QPushButton("Submit",self)
        next_page = MeterConfig()
        submit_button.clicked.connect(lambda: open_next_page(self, next_page))
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

        # Container widget
        container = QWidget(self)
        container.setLayout(main_layout)
        self.setCentralWidget(container)

    def addRow(self):
        self.table.insertRow(self.table.rowCount())
        garbage_checkbox = QTableWidgetItem()
        garbage_checkbox.setFlags(Qt.ItemFlag.ItemIsUserCheckable | Qt.ItemFlag.ItemIsEnabled)
        garbage_checkbox.setCheckState(Qt.CheckState.Unchecked)
        self.table.setItem(self.table.rowCount()-1, 5, garbage_checkbox)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ServerPage()
    window.show()
    sys.exit(app.exec_())
