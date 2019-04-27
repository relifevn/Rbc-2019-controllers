import urllib.request
contents = urllib.request.urlopen("http://192.168.4.1/?pin=12").read()
print('[INFO] contents = ')
print(contents)

