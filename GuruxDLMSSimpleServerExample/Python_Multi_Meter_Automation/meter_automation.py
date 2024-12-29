import subprocess
import threading
import platform
import os
import time
import atexit
import psutil
processes = []

def check_port_status_wsl(port):
    """Checks if the specified port is in use within WSL."""
    try:
        result = subprocess.run(
            ["wsl", "netstat", "-tuln"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        if result.returncode != 0:
            print(f"Error checking port {port} in WSL: {result.stderr}")
            return False
        
        # Split the output into lines and search for the port in LISTEN state
        for line in result.stdout.splitlines():
            if f":{port} " in line or f":{port} " in line.split()[3]:
                print(f"Port {port} found in netstat output: {line}")
                return True
        
        print(f"Port {port} not found in netstat output.")
        return False
    except Exception as e:
        print(f"Error checking port {port} in WSL: {e}")
        return False
def convert_to_wsl_path(path):
    """Converts a Windows path to a WSL-compatible path."""
    if os.name == "nt":  # Check if the system is Windows
        drive, rest = os.path.splitdrive(path)
        rest = rest.replace('\\', '/')  # Perform the replacement outside the f-string
        wsl_path = f"/mnt/{drive.lower()[0]}{rest}"
        return wsl_path.rstrip('/') 
    return path 

def monitor_process(process, server_port):
    """Monitors a process and logs its output."""
    try:
        while process.poll() is None:  # Loop until process terminates
            output = process.stdout.readline()
            if output:
                print(f"[Server on port {server_port}] {output.strip()}")
            error = process.stderr.readline()
            if error:
                print(f"[Error on port {server_port}] {error.strip()}")
    except Exception as e:
        print(f"Error monitoring process {process.pid}: {e}")

def start_servers(binary_path, config_path, num_servers, start_port, start_instance, use_wsl=False):
    """Starts servers on consecutive ports."""
    # Convert binary_path to WSL path if WSL is used
    if use_wsl:
        binary_path = convert_to_wsl_path(binary_path)
        config_path = convert_to_wsl_path(config_path)

    ports = range(start_port, start_port + num_servers)
    instances = range(start_instance, start_instance + num_servers)
    print(f"Starting servers on the following ports: {list(ports)}")

    view_garbage = input("Do you want to enable garbage values? (yes/no): ").strip().lower() == "yes"

    for port, instance in zip(ports, instances):
        print(f"Preparing to start server on port: {port} with instance: {instance}")
        command = [binary_path, "-p", str(port)]

        if view_garbage:
            command += ["-g"]

        command += ["-c", config_path, "-I", str(instance)]

        if use_wsl:
            command = ["wsl"] + command

        print(f"Executing command: {' '.join(command)}")
        env = os.environ.copy()
        # Launch the process
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            env=env,
            preexec_fn=os.setpgrp if os.name != "nt" else None,
            creationflags=subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
        )
        processes.append(process)

        # Start monitoring the process output
        threading.Thread(target=monitor_process, args=(process, port), daemon=True).start()

        # Verify if the server is running
        time.sleep(5)
        if check_port_status_wsl(port):
            print(f"Server on port {port} with instance {instance} is running.")
        else:
            print(f"Server on port {port} with instance {instance} failed to start.")

    print(f"Started {len(ports)} server(s). Output is being monitored.")

def cleanup():
    """Forcefully terminates all running server processes."""
    print("Initiating cleanup. Terminating all server processes...")
    for process in processes:
        try:
            if process.poll() is None:  # Check if process is still running
                # On Windows, use terminate() directly, on Unix use killpg()
                if os.name != "nt":  # Unix-based system (Linux, macOS)
                    os.killpg(os.getpgid(process.pid), signal.SIGTERM)
                    print(f"Sent SIGTERM to process group {process.pid} on Unix system.")
                else:  # Windows system
                    process.terminate()  # Terminate process on Windows
                    print(f"Sent terminate signal to process {process.pid} on Windows.")
                
                # Use psutil to terminate child processes
                parent_process = psutil.Process(process.pid)
                for child in parent_process.children(recursive=True):
                    print(f"Terminating child process {child.pid}")
                    child.terminate()
                    child.wait(timeout=5)
                
                process.wait(timeout=5)  # Wait for the process to exit
                print(f"Process {process.pid} terminated successfully.")
        except Exception as e:
            print(f"Failed to terminate process {process.pid}: {e}")

atexit.register(cleanup)

if __name__ == "__main__":
    binary_path = input("Enter the path to the binary executable: ")
    config_path = input("Enter the path to the config.json file: ")
    
    try:
        num_servers = int(input("Enter the number of servers to start: "))
        start_port = int(input("Enter the starting port number: "))
        start_instance = int(input("Enter the starting instance number: "))

        is_windows = platform.system() == "Windows"
        use_wsl = is_windows

        print(f"Starting {num_servers} servers from port {start_port} with instances starting at {start_instance}.")
        start_servers(binary_path, config_path, num_servers, start_port, start_instance, use_wsl=use_wsl)
        print("Servers are running.")

        # Keep the script running to allow monitoring
        while True:
            time.sleep(5)
    except KeyboardInterrupt:
        print("\nScript interrupted by user. Shutting down servers.")
        cleanup()
    except ValueError as e:
        print(f"Error: {e}")
    finally:
        print("Exiting script. Ensuring all processes are terminated.")
        cleanup()