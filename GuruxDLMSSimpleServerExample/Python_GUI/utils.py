from PyQt5.QtWidgets import QLabel
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt

# Function to quickly create qlabels
def createLabel(text, fontsize=16, color="black",alignment = True):
    label = QLabel(text)
    label.setFont(QFont("Arial", fontsize, QFont.Bold))
    label.setStyleSheet(f"color: {color};")
    if (alignment == True):
        label.setAlignment(Qt.AlignCenter)
    return label

def open_next_page(self, page):
       """Open the next page and close the current window."""
       self.next_page = page  # Create an instance of the next page
       self.next_page.show()  # Show the next page
       self.close()