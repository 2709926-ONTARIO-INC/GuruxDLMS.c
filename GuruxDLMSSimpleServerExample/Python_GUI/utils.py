from PyQt5.QtWidgets import QLabel, QPushButton
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt

# Function to quickly create qlabels
def createLabel(text, fontsize=16, color="black", alignment = True):
    label = QLabel(text)
    label.setFont(QFont("Arial", fontsize, QFont.Bold))
    label.setStyleSheet(f"color: {color};")
    if (alignment == True):
        label.setAlignment(Qt.AlignCenter)
    return label

def createButton(text, fontsize=12):
    button = QPushButton(text)
    button.setFont(QFont("Arial", fontsize))
    button.setMinimumWidth(100)
    button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px; padding:8px")
    return button

def open_next_page(self, page):
    """Open the next page and close the current window."""
    self.next_page = page  # Create an instance of the next page
    self.next_page.show()  # Show the next page
    self.close()

def open_previous_page(self, page):
    self.prev_page = page
    self.prev_page.show()
    self.close() 