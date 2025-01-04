from PyQt5.QtWidgets import QApplication, QWidget, QLineEdit, QPushButton, QVBoxLayout, QHBoxLayout, QMainWindow, QMessageBox
from PyQt5.QtGui import QFont, QRegExpValidator
from PyQt5.QtCore import Qt, QRegExp
from utils import createLabel, open_next_page, open_previous_page

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
        container.setMaximumSize(600,200)

        # Layouts
        layout = QVBoxLayout(container)
        layout.setAlignment(Qt.AlignCenter)

        # Title label
        title_label = createLabel("Welcome!")

        # User ID field
        user_id_layout = QHBoxLayout()
        user_id_label = createLabel("User ID:", 12, alignment=False)
        user_id_input = QLineEdit()
        user_id_input.setFont(QFont("Arial", 12))
        user_id_input.setMaximumWidth(300)
        user_id_input.setStyleSheet("background-color: white; border: 1px solid black;")
        user_regex = QRegExp("admin")
        user_validator = QRegExpValidator(user_regex, user_id_input)
        user_id_input.setValidator(user_validator)
        user_id_layout.addWidget(user_id_label)
        user_id_layout.addWidget(user_id_input)

        # Password field
        password_layout = QHBoxLayout()
        password_label = createLabel("Password:", 12, alignment=False)
        password_input = QLineEdit()
        password_input.setFont(QFont("Arial", 12))
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

        back_btn = QPushButton("Back", self)
        back_btn.clicked.connect(lambda: self.openPrevPage())
        back_btn.setFont(QFont("Arial", 12))
        back_btn.setMinimumWidth(100)
        back_btn.setStyleSheet(
            "background-color: white; border: 1px solid black; border-radius: 5px;"
        )

        # Submit button
        submit_button = QPushButton("Login",self)
        submit_button.clicked.connect(lambda: validateInput())
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;")

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(back_btn)
        button_layout.addWidget(submit_button)
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