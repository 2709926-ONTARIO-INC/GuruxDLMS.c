from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QMainWindow, QLineEdit, QTableWidget, QTableWidgetItem, QPushButton, QComboBox, QScrollArea, QHeaderView
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, open_previous_page, createButton
import json
import os

class Meter(QWidget):
    def __init__(self,config):
        super().__init__()
        self.createMeter(config)
    
    def createMeter(self, config):
        self.setMaximumSize(500, 610)
        meter_layout = QVBoxLayout()
        meter_layout.setAlignment(Qt.AlignCenter) 

        meter_label = createLabel(config["meter_type"])
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
                item_input.addItem(config["manufacturer"])
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
                item_input.setText(str(config["no_of_meters"]))
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
        table.setRowCount(5)
        table.setColumnCount(3)
        table.setHorizontalHeaderLabels(["Qty.","Min","Max"])
        table.verticalHeader().setVisible(False)
        table.setEditTriggers(QTableWidget.NoEditTriggers)
        table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        labels = ["V", "I", "P.F", "f", "Block Load"]
        json_keys = ["voltage_limits", "current_limits", "frequency_limits", "power_factor_limits", "block_energy_limits"]
        limits = ["lower_limit", "upper_limit"]
        for row in range(table.rowCount()):
            for col in range(table.columnCount()):
                if col == 0:
                    item = QTableWidgetItem(labels[row])
                else: 
                    if config["meter_type"] == "Single Phase":
                        if row == 1:
                            item = QTableWidgetItem(str(config[json_keys[row]]["phase_current"][limits[col-1]]/1000))
                        elif row == 4:
                            item = QTableWidgetItem(str(config[json_keys[row]]["kWh_import"][limits[col-1]]/100))
                        else:
                            item = QTableWidgetItem(str(config[json_keys[row]][limits[col-1]]/1000))
                    else:
                        if row == 2:
                            item = QTableWidgetItem(str(config[json_keys[row]][limits[col-1]]/1000))
                        elif row == 4:
                            item = QTableWidgetItem(str(config[json_keys[row]]["kWh_import"][limits[col-1]]/100))
                        else:
                            multiplier = 100000 if json_keys[row] == "current_limits" else 1000
                            item = QTableWidgetItem(str(config[json_keys[row]]["L1"][limits[col-1]]/multiplier))
                item.setTextAlignment(Qt.AlignCenter)  
                table.setItem(row, col, item)
            meter_layout.addWidget(table)

        table.setFixedHeight(250)  
        table.setFixedWidth(475)
            
        self.setLayout(meter_layout)
class MeterConfig(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Meter Config')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # Layouts
        main_widget = QWidget()
        main_layout = QVBoxLayout()
        self.setCentralWidget(main_widget)
        main_widget.setLayout(main_layout)
        meter_layout = QHBoxLayout()
        meter_layout.setAlignment(Qt.AlignLeft)

        config_folder = r"GuruxDLMSSimpleServerExample/Python_GUI/Config"
        meter_configs = self.load_meter_configs(config_folder)

        for config in meter_configs:
            meter = Meter(config)
            meter_layout.addWidget(meter)

        # Container widget for the scroll area
        container_widget = QWidget()
        container_widget.setLayout(meter_layout)

        #Scroll Area
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setWidget(container_widget)

        main_layout.addWidget(scroll)

        back_btn = createButton("Back")
        back_btn.clicked.connect(lambda: self.openPrevPage())

        # Next button
        next_button = createButton("Next")
        next_button.clicked.connect(lambda: self.openNextPage())

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        button_layout.addWidget(back_btn)
        button_layout.addWidget(next_button)
        main_layout.addLayout(button_layout)

    def openNextPage(self):
        from eventsSimulation import EventSimulationApp
        next_page = EventSimulationApp()
        open_next_page(self, next_page)

    def openPrevPage(self):
        from ServerPage import ServerPage
        prev_page = ServerPage()
        open_previous_page(self, prev_page)

    def load_meter_configs(self, config_folder):
        meter_configs = []
        for filename in os.listdir(config_folder):
            if filename.endswith(".json"):
                file_path = os.path.join(config_folder, filename)
                with open(file_path, 'r') as f:
                    meter_config = json.load(f)
                    meter_configs.append(meter_config)
        return meter_configs

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    meter_config = MeterConfig()
    meter_config.show()
    sys.exit(app.exec_())
