from PyQt5.QtWidgets import QMessageBox, QCheckBox
from app_state import state
import json
import os

class SaveConfigJson():
    def save_to_json_config(self, input_table):
            meter_type_for_file_path = ""

            # Function to check if input is a valid integer
            def is_valid_input(item):
                if item and item.text():  
                    text = item.text()
                    try:
                        float(text) 
                        return True
                    except ValueError:
                        return False
                return False

            # Check if all fields have valid input
            for row in range(5):
                for col in range(1, 3):
                    item = input_table.item(row, col)
                    if not is_valid_input(item):
                        QMessageBox.warning(self, "Input Error", "Please enter valid values.")
                        return
                
            current_row = self.parent_table.currentRow()
            if current_row == -1:
                QMessageBox.warning(self, "Selection Error", "Please select a row to save the data.")
                return
            meter_type = self.parent_table.cellWidget(current_row, 0).currentText()
            manufacturer = self.parent_table.cellWidget(current_row, 2).currentText()
            no_of_meters = int(self.parent_table.item(current_row, 1).text()) if self.parent_table.item(current_row, 1).text() != "" else 0

            # Save data to global state
            row_data = {
                "voltage_limits": {
                    "min": input_table.item(0, 1).text(),
                    "max": input_table.item(0, 2).text()
                },
                "current_limits": {
                    "min": input_table.item(1, 1).text(),
                    "max": input_table.item(1, 2).text()
                },
                "power_factor_limits": {
                    "min": input_table.item(2, 1).text(),
                    "max": input_table.item(2, 2).text()
                },
                "frequency_limits": {
                    "min": input_table.item(3, 1).text(),
                    "max": input_table.item(3, 2).text()
                },
                "block_load_limits": {
                    "min": input_table.item(4, 1).text(),
                    "max": input_table.item(4, 2).text()
                },
                'meter_type': self.parent_table.cellWidget(current_row, 0).currentText() if self.parent_table.cellWidget(current_row, 0) else "",
                'num_meters': self.parent_table.item(current_row, 1).text() if self.parent_table.item(current_row, 1) else "",
                'manufacturer': self.parent_table.cellWidget(current_row, 2).currentText() if self.parent_table.cellWidget(current_row, 2) else "",
                'start_port': self.parent_table.item(current_row, 3).text() if self.parent_table.item(current_row, 3) else "",
                'start_instance': self.parent_table.item(current_row, 4).text() if self.parent_table.item(current_row, 4) else "",
                'garbage_enabled': self.parent_table.cellWidget(current_row, 5).findChild(QCheckBox).isChecked() if self.parent_table.cellWidget(current_row, 5) else False
            }

            state.row_data[current_row] = row_data

            print(state.row_data)

            # Convert Meter Type for file name
            file_path_name = {"Single Phase": "single_phase", "Three Phase (WC)": "three_phase_wc", "Three Phase (LTCT)": "three_phase_ltct", "Three Phase (HTCT)": "three_phase_htct"}
            if meter_type:
                meter_type_for_file_path = file_path_name[meter_type]
            
            # Get the directory of the current script
            script_directory = os.path.dirname(os.path.abspath(__file__))

            # Construct the config file path
            config_file_name = fr"Config\{manufacturer.lower().replace(' ', '_')}_{meter_type_for_file_path}_config.json"

            # Prepend the full directory path
            config_file_path = os.path.join(script_directory, config_file_name)

            json_keys = ["no_of_meters", "meter_type", "manufacturer", "voltage_limits", "current_limits", "frequency_limits", "power_factor_limits", "block_energy_limits"]
            phases = ["L1", "L2", "L3"]
            limits = ["lower_limit", "upper_limit"]
            
            config_data = {}

            # Loop to dynamically insert keys in config_data
            for i, key in enumerate(json_keys):
                # First three static keys
                if key == "no_of_meters":
                    config_data[key] = no_of_meters
                elif key == "meter_type":
                    config_data[key] = meter_type
                elif key == "manufacturer":
                    config_data[key] = manufacturer
                else:
                    # Keys about meter values
                    for j, limit in enumerate(limits):
                        if key not in config_data:
                            config_data[key] = {}
                        # For Single Phase
                        if meter_type == "Single Phase":
                            # For current
                            if key == "current_limits":
                                subkeys = ["phase_current", "neutral_current"]
                                for subkey in subkeys:
                                    if subkey not in config_data[key]:
                                        config_data[key][subkey] = {}
                                    config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                            # For block load
                            elif key == "block_energy_limits":
                                subkeys = ["kWh_import", "kWh_export"]
                                for subkey in subkeys:
                                    if subkey not in config_data[key]:
                                        config_data[key][subkey] = {}
                                    config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 100)
                            # For Signed power factor
                            elif key == "power_factor_limits":
                                del config_data[key]
                                if "signed_" + key not in config_data:      
                                    config_data["signed_" + key] = {}
                                config_data["signed_"+key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                            # Other keys
                            else:
                                config_data[key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                        # For Three phase
                        else:
                            # For frequency
                            if key == "frequency_limits":
                                config_data[key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                            # For block load
                            elif key == "block_energy_limits":
                                subkeys = ["kWh_import", "kVAh_lag", "kVAh_lead", "kVAh_import"]
                                for subkey in subkeys:
                                    if subkey not in config_data[key]:
                                        config_data[key][subkey] = {}
                                    config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 100)
                            # Other keys
                            else:
                                for phase in phases:
                                    # Current Multiplier
                                    multiplier = 100000 if key == "current_limits" else 1000
                                    if phase not in config_data[key]:
                                        config_data[key][phase] = {}
                                    config_data[key][phase][limit] = int(float(input_table.item(i-3, j+1).text()) * multiplier)

            # Save the configuration to a new JSON file
            with open(config_file_path, "w") as f:
                json.dump(config_data, f, indent=4)