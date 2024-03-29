import spotipy, requests, os, json, base64, random, time
from datetime import datetime
from spotipy.oauth2 import SpotifyOAuth
from html2image import Html2Image
from PIL import Image

with open('config.json', 'r') as f:
    config_data = json.load(f)

# Spotify App
client_id               = config_data['spotify_app']['client_id']
client_secret           = config_data['spotify_app']['client_secret']
redirect_uri            = "http://example.com/callback"
scope                   = "user-library-read user-read-currently-playing"

# Open Weather Key
open_weather_api_key    = config_data['open_weather']['api_key']
weather_location_id     = config_data['open_weather']['locations'][0]['id']

# Last Refresh - Refresh if anything is changed or got to its delay
last_weather_icon       = config_data['smartframe_app']['last_refresh']['weather_icon']
last_weather_temp       = config_data['smartframe_app']['last_refresh']['weather_temp']
last_image              = config_data['smartframe_app']['last_refresh']['image']
last_song               = config_data['smartframe_app']['last_refresh']['song']
last_timestamp          = config_data['smartframe_app']['last_refresh']['timestamp']
refresh_delay           = config_data['smartframe_app']['refresh_delay']
target_timestamp        = last_timestamp + refresh_delay

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

def get_current_song() -> (bool, str, str, str):
    sp = spotipy.Spotify(auth_manager=SpotifyOAuth(scope=scope, client_id=client_id, client_secret=client_secret, redirect_uri=redirect_uri))
    results = sp.current_user_playing_track()

    if results == None:
        return (False, "", "", "")

    song_name = results['item']['name']
    artist_name = results['item']['album']['artists'][0]['name']
    song_image = results['item']['album']['images'][0]['url']
    
    return (results['is_playing'], song_name, artist_name, song_image)

def get_current_weather(location_id) -> (str, str):
    URL = 'http://api.openweathermap.org/data/2.5/weather'
    PARAMS = {
        'id': location_id,
        'units': 'metric',
        'appid': open_weather_api_key
    }

    r = requests.get(URL, params=PARAMS)

    if r.status_code != 200:
        return ("", "")

    weather_temp = str(round(r.json()['main']['temp'])) + "°C"
    weather_code = r.json()['weather'][0]['icon']

    if weather_code in icon_map:
        return (weather_temp, icon_map[weather_code])
    
    return (weather_temp, "wi-cloud")

def get_html_song(song, artist, img, weather_temp, weather_icon):
    return """
    <html style="background-color: #FFF;width: 600px; height: 800px;">
    <body style="font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;">
        <div style="position:absolute; top: 100; left: 140">
            <img style="width: 320px; height: 320px; box-shadow: 0px 10px 10px #AAA" src="{}" alt="">
        </div>
        <div style="position: absolute; top: 490px; width: 600px">
            <p style="text-align: center; font-size: 50px; color: #2A2A2A; margin: 0;">{}</p>
            <p style="text-align: center; font-size: 25px; color: #565656; margin: 0; margin-top: 10px">{}</p>
        </div>

        <div style="color: #FFF; position: absolute; top: 650px; width: 600px; display: flex; align-items: center; justify-content: center;">
            <div style="text-align: center; font-size: 50px; color: #2A2A2A;">
                <p style="margin: 0; ">{}</p>
            </div>
            <div>
                <img style="width: 64px; height: 64px" src="https://raw.githubusercontent.com/erikflowers/weather-icons/master/svg/{}.svg" alt="">
            </div>
        </div>
    </body>
    </html>
    """.format(img, song, artist, weather_temp, icon)

def get_html_showcase(weather_temp, weather_icon):
    # Gets random image from the ./imgs directory
    random_file = "./imgs/" + random.choice(os.listdir("./imgs"))
    data = open(random_file, 'rb').read() # read bytes from file
    data_base64 = base64.b64encode(data)  # encode to base64 (bytes)
    data_base64 = data_base64.decode()    # convert bytes to string

    return """
<html style="background-color: #FFF;width: 600px; height: 800px;">
<body style="font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;">
    <img src="data:image/jpeg;base64,{}" style="width: 600px; height: 800px; object-fit: cover ">
    <div style="border-radius: 0px 0px 0px 20px; margin-left: 375px; background-color: #FFF;position: absolute; top: 15px; width: 200px; display: flex; align-items: center; justify-content: center;">
        <div style="text-align: center; font-size: 50px; color: #2A2A2A;">
            <p style="margin: 0; ">{}</p>
        </div>
        <div>
            <img style="width: 64px; height: 64px; padding-top: 15px" src="https://raw.githubusercontent.com/erikflowers/weather-icons/master/svg/{}.svg" alt="">
        </div>
    </div>
</body>
</html>
    """.format(data_base64, weather_temp, icon)

# Image Values
print("Fetching song from Spotify")
playing_song, song, artist, img = get_current_song()
current_time = datetime.now().strftime("%H:%M")
temperature, icon = get_current_weather(weather_location_id)

if (song == last_song and img == last_image and icon == last_weather_icon and temperature == last_weather_temp):
    if (time.time() < target_timestamp):
        quit()

config_data['smartframe_app']['last_refresh']['weather_icon'] = icon
config_data['smartframe_app']['last_refresh']['weather_temp'] = temperature
config_data['smartframe_app']['last_refresh']['image'] = img
config_data['smartframe_app']['last_refresh']['song'] = song
config_data['smartframe_app']['last_refresh']['timestamp'] = time.time()

with open('config.json', 'w') as f:
    json.dump(config_data, f)

# Determine HTML Code
html_code = ""
if playing_song:
    html_code = get_html_song(song, artist, img, temperature, icon)
else:
    html_code = get_html_showcase(temperature, icon)

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

