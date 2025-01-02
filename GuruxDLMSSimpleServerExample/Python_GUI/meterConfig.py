from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QMainWindow, QLineEdit, QTableWidget, QTableWidgetItem, QPushButton, QComboBox, QScrollArea, QHeaderView
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from simulationPage import SimulationDetailsApp
from utils import createLabel, open_next_page
import json

class Meter(QWidget):
    def __init__(self, metertype, total, manufacturer):
        super().__init__()
        self.createMeter(metertype, total, manufacturer)
    
    def createMeter(self, metertype, total, manufacturer):
        meter_layout = QVBoxLayout()
        meter_layout.setAlignment(Qt.AlignCenter) 

        meter_label = createLabel(metertype)
        meter_label.setStyleSheet("background-color: lightblue; border: 1px solid black;")
        meter_layout.addWidget(meter_label)
        meter_layout.setSpacing(50)

        Parameters = ["Total", "Manufacturer", "Frequency", "Model", "Meter Constant", "Communication"]

        for item in Parameters:
            itembox = QHBoxLayout()

            item_label = QLabel(item)
            item_label.setFont(QFont("Arial", 12))

            if (item == "Model"):
                item_input = QComboBox()
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(200)
            elif (item == "Manufacturer"): 
                item_input = QComboBox()
                item_input.addItem(manufacturer)
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(200)
            elif (item == "Frequency"): 
                item_input = QComboBox()
                item_input.addItems(["50 Hz","60 Hz"])
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(200)
            elif (item == "Total"): 
                item_input = QLineEdit()
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(200)
                item_input.setText(total)
            else: 
                item_input = QLineEdit()
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(200)

            itembox.addWidget(item_label)
            itembox.addWidget(item_input)
            meter_layout.addLayout(itembox)
            meter_layout.setSpacing(20)

        table = QTableWidget()
        # Table Styling and Resizing
        table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
        )
        table.setRowCount(4)
        table.setColumnCount(3)
        table.setHorizontalHeaderLabels(["Qty.","Min","Max"])
        table.verticalHeader().setVisible(False)
        table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        table.setItem(0,0, QTableWidgetItem("V"))
        table.setItem(1,0, QTableWidgetItem("I"))
        table.setItem(2,0, QTableWidgetItem("P.F"))
        table.setItem(3,0, QTableWidgetItem("f"))
        meter_layout.addWidget(table)

        button_bar = QHBoxLayout()
        save_btn = QPushButton("Save")
        save_btn.setFont(QFont("Arial", 12))
        save_btn.setStyleSheet("background-color: lightblue; border: 1px solid black;")

        edit_btn = QPushButton("Edit")
        edit_btn.setFont(QFont("Arial", 12))
        edit_btn.setStyleSheet("background-color: lightblue; border: 1px solid black;")

        delete_btn = QPushButton("Delete")
        delete_btn.setFont(QFont("Arial", 12))
        delete_btn.setStyleSheet("background-color: lightblue; border: 1px solid black;")

        button_bar.addWidget(save_btn)
        button_bar.addWidget(edit_btn)
        button_bar.addWidget(delete_btn)
        meter_layout.addLayout(button_bar)
            
        self.setLayout(meter_layout)

class MeterConfig(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Meter Config')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")

        # Layouts
        main_widget = QWidget()
        main_layout = QVBoxLayout()
        self.setCentralWidget(main_widget)
        main_widget.setLayout(main_layout)
        meter_layout = QHBoxLayout()
        meter_layout.setAlignment(Qt.AlignCenter) 

        f =  open("simulation.json")
        meter_data = json.load(f)

        for i in meter_data:
            meter = Meter(i[1], i[2], i[3])
            meter_layout.addWidget(meter)

        # Container widget for the scroll area
        container_widget = QWidget()
        container_widget.setLayout(meter_layout)

        #Scroll Area
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setWidget(container_widget)
        main_layout.addWidget(scroll)

        # Submit button
        submit_button = QPushButton("Submit",self)
        next_page = SimulationDetailsApp()
        submit_button.clicked.connect(lambda: open_next_page(self, next_page))
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    meter_config = MeterConfig()
    meter_config.show()
    sys.exit(app.exec_())
