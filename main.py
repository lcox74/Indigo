import spotipy, requests, os, json
from datetime import datetime
from spotipy.oauth2 import SpotifyOAuth
from html2image import Html2Image
from PIL import Image

config_file = open('config.json', 'r')
config_data = json.load(config_file)
config_file.close()

# Spotify App
client_id               = config_data['spotify_app']['client_id']
client_secret           = config_data['spotify_app']['client_secret']
redirect_uri            = "http://example.com/callback"
scope                   = "user-library-read user-read-currently-playing"

# Open Weather Key
open_weather_api_key    = config_data['open_weather']['api_key']
weather_location_id     = config_data['open_weather']['locations'][0]['id']

# OpenWeather to https://erikflowers.github.io/weather-icons/
icon_map = {
    "11d": "wi-day-thunderstorm",
    "11n": "wi-night-thunderstorm",
    "09d": "wi-day-showers",
    "09n": "wi-night-showers",
    "10d": "wi-day-rain",
    "10n": "wi-night-rain",
    "13d": "wi-day-snow",
    "13n": "wi-night-snow",
    "50d": "wi-day-haze",
    "50n": "wi-night-fog",
    "01d": "wi-day-sunny",
    "01n": "wi-night-clear",
    "02d": "wi-day-cloudy",
    "02n": "wi-night-alt-cloudy",
    "03d": "wi-day-cloudy",
    "03n": "wi-night-alt-cloudy",
    "04d": "wi-day-cloudy",
    "04n": "wi-night-alt-cloudy",
}

def get_current_song() -> (str, str, str):
    sp = spotipy.Spotify(auth_manager=SpotifyOAuth(scope=scope, client_id=client_id, client_secret=client_secret, redirect_uri=redirect_uri))
    results = sp.current_user_playing_track()

    song_name = results['item']['name']
    artist_name = results['item']['album']['artists'][0]['name']
    song_image = results['item']['album']['images'][0]['url']
    
    return (song_name, artist_name, song_image)

def get_current_weather_icon(location_id) -> str:
    URL = 'http://api.openweathermap.org/data/2.5/weather'
    PARAMS = {
        'id': location_id,
        'units': 'metric',
        'appid': open_weather_api_key
    }

    r = requests.get(URL, params=PARAMS)
    weather_code = r.json()['weather'][0]['icon']

    if weather_code in icon_map:
        return icon_map[weather_code]
    
    return "wi-cloud"

# Image Values
print("Fetching song from Spotify")
song, artist, img = get_current_song()
current_time = datetime.now().strftime("%H:%M")
icon = get_current_weather_icon(weather_location_id)

html_code = """
<html style="background-color: #FFF;width: 600px; height: 800px;">
<body style="font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;">
    <div style="position:absolute; top: 100; left: 140">
        <img style="width: 320px; height: 320px; box-shadow: 0px 10px 10px #AAA" src="{}" alt="">
    </div>
    <div style="position: absolute; top: 490px; width: 600px">
        <p style="text-align: center; font-size: 50px; color: #2A2A2A; margin: 0;">{}</p>
        <p style="text-align: center; font-size: 25px; color: #565656; margin: 0; margin-top: 10px">{}</p>
    </div>

    <div style="position: absolute; top: 650px; width: 600px; display: flex; align-items: center; justify-content: center;">
        <div style="text-align: center; font-size: 50px; color: #2A2A2A;">
            <p style="margin: 0; ">{}</p>
        </div>
        <div>
            <img style="width: 100px; height: 100px" src="https://github.com/erikflowers/weather-icons/blob/master/svg/{}.svg" alt="">
        </div>
    </div>
</body>
</html>
""".format(img, song, artist, current_time, icon)


print("Constructing ePaper Graphic")
hti = Html2Image()
hti.screenshot(html_str=html_code, save_as='image.png', size=(600, 800))

# Temporarily converting png to jpg
temp_png = Image.open('image.png')
temp_png.load()

temp_jpg = Image.new("RGB", temp_png.size, (255, 255, 255))
temp_jpg.paste(temp_png, mask=temp_png.split()[3])
temp_jpg.save('image.jpg', 'JPEG', quality=80)

print("Writing to display")
os.system('sudo ./smartframe.out')

