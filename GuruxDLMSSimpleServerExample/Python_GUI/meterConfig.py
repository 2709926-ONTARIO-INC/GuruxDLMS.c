from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QMainWindow, QLineEdit, QTableWidget, QTableWidgetItem, QComboBox, QScrollArea, QHeaderView, QGridLayout
from PyQt5.QtGui import QFont, QIcon
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, open_previous_page, createButton
from resource_path import resource_path
import json
import random
import os

class Meter(QWidget):
    def __init__(self,config):
        super().__init__()
        self.createMeter(config)
    
    def createMeter(self, config):
        screensize = QApplication.primaryScreen().size()
        self.setMaximumSize((screensize.width()-125)//4, 610)
        meter_layout = QVBoxLayout()
        meter_layout.setAlignment(Qt.AlignCenter) 

        meter_label = createLabel(config["meter_type"])
        meter_label.setStyleSheet("background-color: lightblue; border: 1px solid black;")
        meter_layout.addWidget(meter_label)
        meter_layout.setSpacing(50)

        Parameters = ["Total", "Manufacturer", "Frequency", "Model", "Meter Constant", "Communication"]

        # Company models (change these to actual models for each company)
        model_data = {
            "Genus": {
                "Single Phase": ["Saksham 145"],
                "Three Phase": ["Saksham 345", "Saksham-340 LTCT", "Saksham-340 HTCT"]
            },
            "HPL": {
                "Single Phase": ["SPSM152G111000ME00"],
                "Three Phase": ["TLSM282I131000OC00"]
            },
            "AEW": {
                "Single Phase": ["AT-121"],
                "Three Phase": ["AEW100-D36"]
            },
            "Secure Meters Ltd.": {
                "Single Phase": ["Liberty 200", "Sprint 231"],
                "Three Phase": ["Premier 211", "Premier 520"]
            }
        }

        for item in Parameters:
            itembox = QHBoxLayout()

            item_label = QLabel(item)
            item_label.setFont(QFont("Arial", 12))

            if (item == "Manufacturer"): 
                item_input = QLineEdit()
                item_input.setReadOnly(True)
                item_input.setText(config["manufacturer"])
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)
            elif (item == "Frequency"): 
                item_input = QComboBox()
                item_input.addItems(["50 Hz","60 Hz"])
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)
            elif (item == "Total"): 
                item_input = QLineEdit()
                item_input.setReadOnly(True)
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)
                item_input.setText(str(config["no_of_meters"]))
            elif (item == "Meter Constant"): 
                a = [500, 600, 800, 1000]
                item_input = QLineEdit()
                value = random.sample(a,1)
                item_input.setReadOnly(True)
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)
                item_input.setText(str(value[0]))
            elif (item == "Model"): 
                # Get the manufacturer and meter type from config
                manufacturer = config["manufacturer"]
                meter_type = config["meter_type"]

                if "Three Phase" in meter_type:
                    meter_type = "Three Phase"
                
                # Fetch the list of models based on manufacturer and meter type
                models = model_data.get(manufacturer, {}).get(meter_type, [])

                # Randomly select a model from the available models
                if models:
                    model = random.choice(models)
                else:
                    model = "Unknown Model"

                # Create a QLineEdit for the model
                item_input = QLineEdit()
                item_input.setReadOnly(True)
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)
                item_input.setText(model)
            else: 
                item_input = QLineEdit()
                item_input.setReadOnly(True)
                item_input.setFont(QFont("Arial", 12))
                item_input.setStyleSheet("background-color: white;")
                item_input.setFixedWidth(250)

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
        table.setHorizontalHeaderLabels(["Parameters","Min","Max"])
        table.verticalHeader().setVisible(False)
        table.setEditTriggers(QTableWidget.NoEditTriggers)
        table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        labels = ["Voltage (V)", "Current (A)", "Power Factor", "Frequency (Hz)", "Block Load (kWh)"]
        json_keys = ["voltage_limits", "current_limits", "frequency_limits", "power_factor_limits", "block_energy_limits"]
        limits = ["lower_limit", "upper_limit"]
        for row in range(table.rowCount()):
            for col in range(table.columnCount()):
                if col == 0:
                    item = QTableWidgetItem(labels[row])
                else: 
                    if config["meter_type"] == "Single Phase":
                        if row == 1:
                            item = QTableWidgetItem(str(int(config[json_keys[row]]["phase_current"][limits[col-1]]/1000)))
                        elif row == 4:
                            item = QTableWidgetItem(str(int(config[json_keys[row]]["kWh_import"][limits[col-1]])))
                        elif row == 3:
                            item = QTableWidgetItem(str(config["signed_"+json_keys[row]][limits[col-1]]/1000))
                        elif row == 0:
                            item = QTableWidgetItem(str(int(config[json_keys[row]][limits[col-1]]/1000)))
                        else:
                            item = QTableWidgetItem(str(config[json_keys[row]][limits[col-1]]/1000))
                    else:
                        if row == 2:
                            item = QTableWidgetItem(str(config[json_keys[row]][limits[col-1]]/1000))
                        elif row == 4:
                            item = QTableWidgetItem(str(int(config[json_keys[row]]["kWh_import"][limits[col-1]])))
                        elif row == 0 or row == 1:
                            multiplier = 100000 if json_keys[row] == "current_limits" else 1000
                            item = QTableWidgetItem(str(int(config[json_keys[row]]["L1"][limits[col-1]]/multiplier)))
                        else:
                            item = QTableWidgetItem(str(config[json_keys[row]]["L1"][limits[col-1]]/1000))
                item.setTextAlignment(Qt.AlignCenter)  
                table.setItem(row, col, item)
            meter_layout.addWidget(table)

        table.setFixedHeight(250)  
        table.setFixedWidth((screensize.width()-180)//4)
            
        self.setLayout(meter_layout)
class MeterConfig(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Meshnet Electronics VM Simulator")
        self.setWindowState(Qt.WindowMaximized)
        self.setStyleSheet("background-color: #F1F1F1;")
        image_path = fr"{resource_path('assets/images/icon.png')}"
        self.setWindowIcon(QIcon(image_path))

        # Layouts
        main_widget = QWidget()
        main_layout = QVBoxLayout()
        self.setCentralWidget(main_widget)
        main_widget.setLayout(main_layout)
        meter_layout = QGridLayout()
        meter_layout.setAlignment(Qt.AlignLeft)
        meter_layout.setSpacing(20)

        config_file_name = f"Config\\"

        # Prepend the full directory path
        config_folder = resource_path(config_file_name)
        meter_configs = self.load_meter_configs(config_folder)

        # Add each meter into the grid layout (4 per row)
        row = 0
        col = 0
        for config in meter_configs:
            meter = Meter(config)
            meter_layout.addWidget(meter, row, col)
            col += 1
            if col == 4: 
                col = 0
                row += 1

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
        full_config_folder = resource_path(config_folder)
        for filename in os.listdir(full_config_folder):
            if filename.endswith(".json"):
                # Construct the full file path
                file_path = os.path.join(full_config_folder, filename)
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
