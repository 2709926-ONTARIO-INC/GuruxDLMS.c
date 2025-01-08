from PyQt5.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QTableWidget, QApplication, QHBoxLayout, QHeaderView, QTableWidgetItem, QCheckBox, QComboBox, QMessageBox, QLabel, QDialog
from PyQt5.QtGui import QIcon, QMovie
from PyQt5.QtCore import Qt, QSize
from utils import createLabel, open_next_page, open_previous_page, createButton
from json_config import SaveConfigJson
from app_state import state
import sys
import os
import meter_automation

class ParameterPopup(QWidget):
    def __init__(self, parent_table, current_row):
        super().__init__()
        self.parent_table = parent_table
        self.current_row = current_row
        self.initUI()

    def initUI(self):
        self.setWindowTitle("KiGG VM Simulator")
        self.setMinimumSize(500, 310)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowModality(Qt.ApplicationModal)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        image_path = os.path.join(script_dir, "assets", "images", "icon.png")
        self.setWindowIcon(QIcon(image_path))

        input_table = QTableWidget()
        # Table Styling and Resizing
        input_table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
        )
        input_table.setRowCount(5)
        input_table.setColumnCount(3)
        input_table.setHorizontalHeaderLabels(["Parameters","Min","Max"])
        input_table.verticalHeader().setVisible(False)
        input_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        for i, label in enumerate(["Voltage (V)", "Current (A)", "Power Factor", "Frequency (Hz)", "Block Load (Wh)"]):
            item = QTableWidgetItem(label)
            item.setTextAlignment(Qt.AlignCenter)  
            item.setFlags(item.flags() & ~Qt.ItemIsEditable) 
            input_table.setItem(i, 0, item)

        for row in range(5):
            for col in range(1, 3):
                item = QTableWidgetItem("")
                item.setTextAlignment(Qt.AlignCenter)
                input_table.setItem(row, col, item)
            print(self.current_row)
            print(len(state.row_data))
            if len(state.row_data) > 0 and self.current_row < len(state.row_data):
                if row == 0:
                    min_item = QTableWidgetItem(state.row_data[self.current_row]["voltage_limits"]["min"])
                    min_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(0, 1, min_item)

                    max_item = QTableWidgetItem(state.row_data[self.current_row]["voltage_limits"]["max"])
                    max_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(0, 2, max_item)

                if row == 1:
                    min_item = QTableWidgetItem(state.row_data[self.current_row]["current_limits"]["min"])
                    min_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(1, 1, min_item)

                    max_item = QTableWidgetItem(state.row_data[self.current_row]["current_limits"]["max"])
                    max_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(1, 2, max_item)

                if row == 2:
                    min_item = QTableWidgetItem(state.row_data[self.current_row]["power_factor_limits"]["min"])
                    min_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(2, 1, min_item)

                    max_item = QTableWidgetItem(state.row_data[self.current_row]["power_factor_limits"]["max"])
                    max_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(2, 2, max_item)

                if row == 3:
                    min_item = QTableWidgetItem(state.row_data[self.current_row]["frequency_limits"]["min"])
                    min_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(3, 1, min_item)

                    max_item = QTableWidgetItem(state.row_data[self.current_row]["frequency_limits"]["max"])
                    max_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(3, 2, max_item)

                if row == 4:
                    min_item = QTableWidgetItem(state.row_data[self.current_row]["block_load_limits"]["min"])
                    min_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(4, 1, min_item)

                    max_item = QTableWidgetItem(state.row_data[self.current_row]["block_load_limits"]["max"])
                    max_item.setTextAlignment(Qt.AlignCenter)
                    input_table.setItem(4, 2, max_item)

        popup_layout = QVBoxLayout(self)
        popup_layout.addWidget(input_table)

        # Save button
        save_button = createButton("Save")
        save_button.clicked.connect(lambda: self.save_popup_data(input_table))

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(save_button)
        popup_layout.addLayout(button_layout)

    def save_popup_data(self, input_table):
        SaveConfigJson.save_to_json_config(self, input_table)   
        self.close()

class ServerPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("KiGG VM Simulator")
        self.setWindowState(Qt.WindowMaximized)
        self.setStyleSheet("background-color: #F1F1F1;")
        script_dir = os.path.dirname(os.path.abspath(__file__))
        image_path = os.path.join(script_dir, "assets", "images", "icon.png")
        self.setWindowIcon(QIcon(image_path))

        # Main layout
        main_layout = QVBoxLayout()
        main_layout.setAlignment(Qt.AlignCenter)

        title_label = createLabel("Please enter the server details", 20, "darkblue")
        main_layout.addWidget(title_label)

        self.table = QTableWidget(self)
        self.table.setColumnCount(7) 

        self.table.setHorizontalHeaderLabels(["Type of Meter", "No. of Meters", "Manufacturer", "Starting Port No.", "Starting Instance No.", "Garbage Values", "Configure Parameters"])

        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch) 
        self.table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
            "QAbstractItemView::indicator { width: 25px; height: 25px }"
        )
        main_layout.addWidget(self.table)

        if state.row_data:
            for _ in range(len(state.row_data)):
                self.addRow()
                current_row = self.table.rowCount() - 1
                self.table.setItem(current_row, 0, QTableWidgetItem(state.row_data[current_row]['meter_type']))
                self.table.setItem(current_row, 1, QTableWidgetItem(state.row_data[current_row]['num_meters']))
                self.table.setItem(current_row, 2, QTableWidgetItem(state.row_data[current_row]['manufacturer']))
                self.table.setItem(current_row, 3, QTableWidgetItem(state.row_data[current_row]['start_port']))
                self.table.setItem(current_row, 4, QTableWidgetItem(state.row_data[current_row]['start_instance']))
                checkbox_widget = self.table.cellWidget(current_row, 5)
                checkbox = checkbox_widget.findChild(QCheckBox)
                checkbox.setChecked(state.row_data[current_row]['garbage_enabled'])

                for column in range(1, 5):
                    item = self.table.item(current_row, column)
                    if item:
                        item.setTextAlignment(Qt.AlignCenter)

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)

        back_btn = createButton("Back")
        back_btn.clicked.connect(lambda: self.openPrevPage())
        button_layout.addWidget(back_btn)

        add_btn = createButton("Add Row")
        add_btn.clicked.connect(lambda: self.addRow())
        button_layout.addWidget(add_btn)

        # Submit button
        submit_button = createButton("Submit")
        submit_button.clicked.connect(self.submitRows)
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

        next_btn = createButton("Next")
        next_btn.clicked.connect(lambda: self.openNextPage())
        button_layout.addWidget(next_btn)

        # Container widget
        container = QWidget(self)
        container.setLayout(main_layout)
        self.setCentralWidget(container)

    def addRow(self):
        row_index = self.table.rowCount()
        self.table.insertRow(row_index)

        for column in range(self.table.columnCount()):
            if column == 0:  # Type of Meter (Dropdown)
                combobox = QComboBox()
                combobox.addItems(["Single Phase", "Three Phase (WC)", "Three Phase (LTCT)", "Three Phase (HTCT)"])
                combobox.setStyleSheet("QComboBox { text-align: center; }")  
                self.table.setCellWidget(row_index, column, combobox)
            elif column == 2:  
                combobox = QComboBox()
                combobox.addItems(["AEW", "Genus", "HPL", "Secure Meters Ltd."])
                combobox.setStyleSheet("QComboBox { text-align: center; }")  
                self.table.setCellWidget(row_index, column, combobox)
            elif column == 5: 
                # Create a custom widget for the checkbox
                checkbox_widget = QWidget()
                checkbox = QCheckBox()
                checkbox_layout = QHBoxLayout()
                checkbox_layout.addWidget(checkbox)
                checkbox_layout.setAlignment(Qt.AlignCenter)  
                checkbox_layout.setContentsMargins(0, 0, 0, 0) 
                checkbox_widget.setLayout(checkbox_layout)
                self.table.setCellWidget(row_index, column, checkbox_widget)
            elif column == 6: 
                push_btn = createButton("Open", 10)
                print(f"Row Index: {row_index}")
                popup_window = ParameterPopup(self.table, row_index)
                push_btn.clicked.connect(lambda: popup_window.show())
                self.table.setCellWidget(row_index, column, push_btn)
            else:
                item = QTableWidgetItem()
                item.setTextAlignment(Qt.AlignCenter)  
                self.table.setItem(row_index, column, item)

    def submitRows(self):
        servers_started_successfully = 0
        servers_failed_to_start = 0
        binary_file_name = ""
        binary_file_path = ""

        # Create a loading message box
        loading_dialog = QDialog(self)
        loading_dialog.setWindowTitle("KiGG VM Simulator")
        loading_dialog.setFixedSize(300, 100)
        loading_dialog.setModal(True)

        # Layout for the dialog
        dialog_layout = QVBoxLayout(loading_dialog)
        dialog_layout.setAlignment(Qt.AlignCenter)

        # Add loading animation
        loading_label = QLabel("Servers are loading. Please wait...")
        dialog_layout.addWidget(loading_label)

        loading_gif = QLabel()
        script_dir = os.path.dirname(os.path.abspath(__file__))
        gif_path = os.path.join(script_dir, "assets", "gifs", "loading.gif")
        movie = QMovie(gif_path)
        movie.setScaledSize(QSize(32, 32))
        loading_gif.setMovie(movie)
        movie.start()

        dialog_layout.addWidget(loading_gif)
        loading_dialog.show()

        # Loop through the rows of the table
        for row in range(self.table.rowCount()):
            meter_type_for_file_path = ""

            # Extract values from each column
            type_of_meter = self.table.cellWidget(row, 0).currentText() if self.table.cellWidget(row, 0) else ""
            num_meters = int(self.table.item(row, 1).text()) if self.table.item(row, 1).text() != "" else 0
            manufacturer = self.table.item(row, 2).currentText() if self.table.item(row, 2) else ""
            start_port = int(self.table.item(row, 3).text()) if self.table.item(row, 3).text() != "" else 0
            start_instance = int(self.table.item(row, 4).text()) if self.table.item(row, 4).text() != "" else 0
            is_garbage_enabled = self.table.cellWidget(row, 5).findChild(QCheckBox).isChecked() if self.table.cellWidget(row, 5) else False

            # Convert Meter Type for file name
            file_path_name = {"Single Phase": "single_phase", "Three Phase (WC)": "three_phase_wc", "Three Phase (LTCT)": "three_phase_ltct", "Three Phase (HTCT)": "three_phase_htct"}
            if type_of_meter:
                meter_type_for_file_path = file_path_name[type_of_meter]

            # Get the directory of the current script
            script_directory = os.path.dirname(os.path.abspath(__file__))

            # Construct the config file path
            config_file_name = fr"Config\{manufacturer.lower().replace(' ', '_')}_{meter_type_for_file_path}_config.json"

            # Prepend the full directory path
            config_file_path = os.path.join(script_directory, config_file_name)

            # Binary file path
            if type_of_meter == "Single Phase":
                binary_file_name = r"Bin\gurux.dlms.simple.server.single.phase.bin"
            else:
                binary_file_name = r"Bin\gurux.dlms.simple.server.three.phase.bin"

            binary_file_path = os.path.join(script_directory, binary_file_name)

            print(config_file_path)
            print(binary_file_path)

            # Call the start_servers function with extracted values
            temp_num_servers_started, temp_num_servers_failed = meter_automation.start_servers(binary_file_path, config_file_path, num_meters, start_port, start_instance, type_of_meter, use_wsl=True, is_garbage_enabled=is_garbage_enabled)

            servers_started_successfully += temp_num_servers_started
            servers_failed_to_start += temp_num_servers_failed
        
        loading_dialog.close()
        QMessageBox.information(self, "KiGG VM Simulator", f"{servers_started_successfully} servers started successfully, {servers_failed_to_start} servers failed to start.")

    def openNextPage(self):
        from meterConfig import MeterConfig
        next_page = MeterConfig()
        open_next_page(self, next_page)

    def openPrevPage(self):
        from loginPage import LoginPage
        prev_page = LoginPage()
        open_previous_page(self, prev_page)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ServerPage()
    window.show()
    sys.exit(app.exec_())
