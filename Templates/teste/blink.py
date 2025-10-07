# Exemplo de código em Python para gravação
import board
import busio
import sdcard
import digitalio
import storage

# Configuração dos pinos SPI e SD
spi = busio.SPI(board.GP18, MOSI=board.GP19, MISO=board.GP16)
cs = digitalio.DigitalInOut(board.GP13)
sd = sdcard.SDCard(spi, cs)
vfs = storage.VfsFat(sd)

# Monta o sistema de arquivos e escreve no cartão
storage.mount(vfs, "/sd")
with open("/sd/meu_arquivo.txt", "w") as f:
    f.write("Olá, dados gravados no cartão SD!\n")

# Desmonta o sistema de arquivos
storage.unmount("/sd")
