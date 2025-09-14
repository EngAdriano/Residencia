import socket

HOST = '0.0.0.0'       # Escuta em todas as interfaces
PORT = 12345           # Porta que o Pico W vai usar

def iniciar_servidor():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print(f"Servidor escutando em {HOST}:{PORT}")

        while True:
            conn, addr = s.accept()
            with conn:
                print(f"Conexão de {addr}")
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    print("Recebido:", data.decode().strip())

if __name__ == "__main__":
    iniciar_servidor()