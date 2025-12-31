import os
import hashlib
from flask import Flask, send_from_directory, jsonify
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

app = Flask(__name__)

KEY = b'1234567890123456'
IV  = b'abcdefghijklmnop'

FILE_NAME = 'firmware.bin'
ENCRYPTED_FILE = 'firmware_encrypted.bin'

def encrypt_firmware():
    with open(FILE_NAME, 'rb') as f:
        data = f.read()

    sha256_hash = hashlib.sha256(data).hexdigest()

    cipher = AES.new(KEY, AES.MODE_CBC, IV)
    encrypted_data = cipher.encrypt(pad(data, AES.block_size))

    with open(ENCRYPTED_FILE, 'wb') as f:
        f.write(encrypted_data)

    return sha256_hash

current_hash = encrypt_firmware()

@app.route('/meta')
def meta():
    return jsonify({
        "version": "1.0.1",
        "size": str(os.path.getsize(FILE_NAME)),
        "sha256": current_hash
    })

@app.route('/firmware')
def firmware():
    return send_from_directory('.', ENCRYPTED_FILE)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)