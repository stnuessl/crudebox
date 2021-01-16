#!/usr/bin/env python

import configparser
import time

def main():

    start = time.time()
    config = configparser.ConfigParser()
    config.read('./test/config/crudebox.conf')

    end = time.time()

    print('Elapsed {} ms'.format(1000 * (end - start)))
    print('Test: {}'.format(config['font']['path']))

if __name__ == '__main__':
    main()
