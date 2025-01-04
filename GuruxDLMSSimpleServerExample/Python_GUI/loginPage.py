from PyQt5.QtWidgets import QApplication, QWidget, QLineEdit, QVBoxLayout, QHBoxLayout, QMainWindow, QMessageBox
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, open_previous_page, createButton

class LoginPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Login Page')
        self.setFixedSize(1550,725)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # Container widget
        container = QWidget(self)
        container.setStyleSheet("*{background-color: lightblue;} .QWidget{border: 2px solid black;}")
        self.setCentralWidget(container)
        self.setContentsMargins(475, 250, 0, 0)
        container.setMaximumSize(600,210)

        # Layouts
        layout = QVBoxLayout(container)
        layout.setAlignment(Qt.AlignCenter)

        # Title label
        title_label = createLabel("Welcome!")

        # User ID field
        user_id_layout = QHBoxLayout()
        user_id_label = createLabel("User ID:", 14, alignment=False)
        user_id_input = QLineEdit()
        user_id_input.setFont(QFont("Arial", 14))
        user_id_input.setMaximumWidth(300)
        user_id_input.setStyleSheet("background-color: white; border: 1px solid black;")
        user_id_layout.addWidget(user_id_label)
        user_id_layout.addWidget(user_id_input)

        # Password field
        password_layout = QHBoxLayout()
        password_label = createLabel("Password:", 14, alignment=False)
        password_input = QLineEdit()
        password_input.setFont(QFont("Arial", 14))
        password_input.setEchoMode(QLineEdit.Password)
        password_input.setMaximumWidth(300)
        password_input.setStyleSheet("background-color: white; border: 1px solid black;")
        password_layout.addWidget(password_label)
        password_layout.addWidget(password_input)

        # Adding widgets to the layout
        layout.addWidget(title_label)
        layout.addSpacing(10)
        layout.addLayout(user_id_layout)
        layout.addSpacing(10)
        layout.addLayout(password_layout)
        layout.addSpacing(20)

        back_btn = createButton("Back")
        back_btn.clicked.connect(lambda: self.openPrevPage())

        # Submit button
        login_button = createButton("Login")
        login_button.clicked.connect(lambda: validateInput())

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(back_btn)
        button_layout.addWidget(login_button)
        layout.addLayout(button_layout)

        # Ensure container resizes with the window
        container.setLayout(layout)

        def validateInput():
            if (user_id_input.text() == "admin" and password_input.text() == "Admin@123"):
                self.openNextPage()
            else:
                popup = QMessageBox()
                popup.setStandardButtons(QMessageBox.Ok)
                popup.setWindowTitle("Error Message")
                popup.setDefaultButton(QMessageBox.Ok)
                popup.setText("Please enter correct User Id and Password")
                popup.exec_()

    def openNextPage(self):
        from ServerPage import ServerPage
        next_page = ServerPage()
        open_next_page(self, next_page)

    def openPrevPage(self):
        from logoPage import LogoPage
        prev_page = LogoPage()
        open_previous_page(self, prev_page)


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    login_page = LoginPage()
    login_page.show()
    sys.exit(app.exec_())