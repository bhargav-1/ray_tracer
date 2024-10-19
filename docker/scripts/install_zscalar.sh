#!/bin/bash

sudo chmod 644 /usr/local/share/ca-certificates/z_scaler.crt
sudo update-ca-certificates --fresh --verbose
