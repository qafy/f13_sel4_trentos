# Secure Communication - Python Server

We provide you with a simple python server, which in a loop,
- decrypts incoming messages with its private key,
- prints the decrypted message on the terminal, and
- encrypts and resends the message back via a preconfigured public key.

## Dependencies

Please make sure you have the pycryptodome python module installed:
```bash
pip3 install --user pycryptodome
```

## How it works

The server generates an RSA-2048 private/public key pair on first use and stores it in the current directory:
```bash
$ ls
README.md  srv_prvt.pem  srv_pub.pem srv.py
```
If you want to generate a new key, simply delete the `*.pem` files and re-run the server.

The server uses and expects the usage of RSA-2048 keys and PKCS#1 OAEP cipher.

The server needs a `clnt_pub.pem` file to be available when started. This file represents the public key of the connecting client and needs to be exported by **you** from the RPi 3 B+.

The server is configured to listen on the IP address 10.0.0.10 and port 8443. This is the same IP address as already configured from homework task 4 and therefore you should be able to directly connect to the server from the RPi 3 B+.

## Execution

A successful run should look something like this:

```bash
$ ./srv.py 
SRV pub key: b'-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqUtn1+JVbBZOj5B071gD\nGM5Lv6Oz7ebaMBemYF4fqlzFSCKjTZ3ZAD1eQor1/tnnxCJ6OIYQeHKOQcmC+syS\nUONoX7Ujkio107Yayh3WPMprsrBiEitOGIFir1eOJC/TDY2ONw906ZxJQP7ZaKYY\nYlxv9AFpwqbjW8KVG10CGYQf55wbkYukqdO9w3TmixpmOj0D6+LCwq//LgwKsnyt\ndkEwr9BV0w83HCYNPT5Oj8zytN0kyZg/ise/uFfp/QemMfOUm6ndN72z9vovs4sK\nwr2bYlWuQXk/ffdI43aN6M4i0JVFNEjlAJFXI487Z5TCG9xaQnhi0Z+BMTIhBVtb\nVQIDAQAB\n-----END PUBLIC KEY-----'
CLNT pub key: b'-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4DsO/WsPOZ6q00I2o5wu\nMsWUxNNtdu/+iDrNS/ZmLYBCRYE7fzDvrmShr5ExxUGqSel9r226B84/hUJC1aZZ\nUe4LVmd+juY6gjETv2Tz/x2baGj2gnZnlY8one6Y8IdlKjKxIIk6W4TJ+3FgILxU\n3YDDPiUWioJjdf3M+UpHBfY2fLLD4NPWxhM16u4jvSzucDywk4BOGrxyGlY9UsYR\ncGuAnQhU23hC9o3IsRBdd+org28P9k3f6hvPySvv/o1Exv1h7FM0X2bXZi1r2kJ/\n08EYyGGB2o+nhvRX/Esr472AzpdOvt2SIDDXjSCUKy9Wt4h5KEr5HFKnmR6jTZO+\nXwIDAQAB\n-----END PUBLIC KEY-----'
Connection from ('127.0.0.1', 49046)
Decrypted: b'Hello World!\n'
```

## Disclaimer

This is **not** how secure communication should be done in a real-world use-case and merely an academic exercise. This protocol lacks integrity protection, provides no forward secrecy, is not authenticated, …
If you want to implement usable cryptography and secure communication, please take a look at existing protocols and solutions, for example [Mbed TLS](https://www.trustedfirmware.org/projects/mbed-tls/), or the [Noise Protocol Framework](https://noiseprotocol.org/).