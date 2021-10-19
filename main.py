import spotipy, requests
from datetime import datetime
from spotipy.oauth2 import SpotifyOAuth
from html2image import Html2Image


# Spotify App
client_id               = ""
client_secret           = ""
redirect_uri            = "http://example.com/callback"
scope                   = "user-library-read user-read-currently-playing"

# Open Weather Key
open_weather_api_key    = ''
weather_location_id     = ''


def get_current_song() -> (str, str):
    sp = spotipy.Spotify(auth_manager=SpotifyOAuth(scope=scope, client_id=client_id, client_secret=client_secret, redirect_uri=redirect_uri))
    results = sp.current_user_playing_track()

    song_name = results['item']['name']
    artist_name = results['item']['album']['artists'][0]['name']

    song_image = results['item']['album']['images'][0]['url']
    img_data = requests.get(song_image).content
    with open('album.jpg', 'wb') as handler:
        handler.write(img_data)
    
    return (song_name, artist_name)

def get_current_weather_icon(location_id) -> str:
    URL = 'http://api.openweathermap.org/data/2.5/weather'
    PARAMS = {
        'id': location_id,
        'units': 'metric',
        'appid': open_weather_api_key
    }

    r = requests.get(URL, params=PARAMS)
    return r.json()['weather'][0]['icon']



# Image Values
song, artist = get_current_song()
current_time = datetime.now().strftime("%H:%M")
icon = get_current_weather_icon(weather_location_id)

html_code = """<html style="width: 600px; height: 800px;">
<body style="font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;">
    <div style="position:absolute; top: 100; left: 140">
        <img style="width: 320px; height: 320px; box-shadow: 0px 10px 10px #AAA" src="./album.jpg" alt="">
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
            <img src="http://openweathermap.org/img/wn/{}@2x.png" alt="">
        </div>
    </div>
</body>
</html>""".format(song, artist, current_time, icon)


hti = Html2Image()
hti.screenshot(html_str=html_code, save_as='image.jpg', size=(600, 800))