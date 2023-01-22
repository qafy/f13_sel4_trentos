#!/usr/bin/env python3
from os.path import exists
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP
import socket
import sys

SRV_PRVT_KEY = None
CLNT_PUB_KEY = None

def main():
    # generate private key if not already existing
    if not exists('srv_prvt.pem'):
        SRV_PRVT_KEY = RSA.generate(2048)
        with open('srv_prvt.pem', 'wb') as fh:
            fh.write(SRV_PRVT_KEY.export_key())
        with open('srv_pub.pem', 'wb') as fh:
            fh.write(SRV_PRVT_KEY.publickey().export_key())
    else:
        with open('srv_prvt.pem', 'rb') as fh:
            SRV_PRVT_KEY = RSA.import_key(fh.read())

    # import clnt pub key
    if not exists('clnt_pub.pem'):
        print('CLNT_PUB_KEY missing')
        sys.exit(1)
    else:
        with open('clnt_pub.pem', 'rb') as fh:
            CLNT_PUB_KEY = RSA.import_key(fh.read())

    print('SRV pub key: {}'.format(SRV_PRVT_KEY.publickey().export_key()))
    print('CLNT pub key: {}'.format(CLNT_PUB_KEY.publickey().export_key()))

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('10.0.0.10', 8443))
        s.listen(1)
        conn, addr = s.accept()
        with conn:
            print('Connection from {}'.format(addr))
            while True:
                data = conn.recv(4096)
                if not data: break

                # decrypt data received from clnt
                decryptor = PKCS1_OAEP.new(SRV_PRVT_KEY)
                decrypted = decryptor.decrypt(data)
                print('Decrypted: {}'.format(decrypted))

                # encrypt data received for clnt
                encryptor = PKCS1_OAEP.new(CLNT_PUB_KEY)
                encrypted = encryptor.encrypt(decrypted)
                conn.sendall(encrypted)

if __name__ == "__main__":
    main()
