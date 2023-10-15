# pip install -r requirements.txt

import time
from datetime import datetime
import turtle
import cmath
import json
import threading
import requests as rest
import DeepHubClasses as dh
import numpy as np

import paho.mqtt.client as mqtt

distance_a1_a2 = 4
meter2pixel = 100
range_offset = 0.9


def screen_init(width=1200, height=800, t=turtle):
    t.setup(width, height)
    t.tracer(False)
    t.hideturtle()
    t.speed(0)


def turtle_init(t=turtle):
    t.hideturtle()
    t.speed(0)


def draw_line(x0, y0, x1, y1, color="black", t=turtle):
    t.pencolor(color)

    t.up()
    t.goto(x0, y0)
    t.down()
    t.goto(x1, y1)
    t.up()


def draw_fastU(x, y, length, color="black", t=turtle):
    draw_line(x, y, x, y + length, color, t)


def draw_fastV(x, y, length, color="black", t=turtle):
    draw_line(x, y, x + length, y, color, t)


def draw_cycle(x, y, r, color="black", t=turtle):
    t.pencolor(color)

    t.up()
    t.goto(x, y - r)
    t.setheading(0)
    t.down()
    t.circle(r)
    t.up()


def fill_cycle(x, y, r, color="black", t=turtle):
    t.up()
    t.goto(x, y)
    t.down()
    t.dot(r, color)
    t.up()


def write_txt(x, y, txt, color="black", t=turtle, f=('Arial', 12, 'normal')):

    t.pencolor(color)
    t.up()
    t.goto(x, y)
    t.down()
    t.write(txt, move=False, align='left', font=f)
    t.up()


def draw_rect(x, y, w, h, color="black", t=turtle):
    t.pencolor(color)

    t.up()
    t.goto(x, y)
    t.down()
    t.goto(x + w, y)
    t.goto(x + w, y + h)
    t.goto(x, y + h)
    t.goto(x, y)
    t.up()


def fill_rect(x, y, w, h, color=("black", "black"), t=turtle):
    t.begin_fill()
    draw_rect(x, y, w, h, color, t)
    t.end_fill()
    pass


def clean(t=turtle):
    t.clear()


def draw_ui(t):
    write_txt(-300, 230, "UWB Tag Position via MQTT",
              "orange",  t, f=('Arial', 28, 'normal'))
    fill_rect(-400, 200, 800, 30, "orange", t)
    write_txt(-50, 200, "WALL", "orange",  t, f=('Arial', 18, 'normal'))


def draw_uwb_anchor(x, y, txt, range, t):
    r = 20
    fill_cycle(x, y, r, "green", t)
    write_txt(x + r, y, txt + ": " + str(range) + "M",
              "black",  t, f=('Arial', 14, 'normal'))


def draw_uwb_tag(x, y, txt, t):
    pos_x = -250 + int(x * meter2pixel)
    pos_y = 150 - int(y * meter2pixel)
    r = 20
    fill_cycle(pos_x, pos_y, r, "blue", t)
    write_txt(pos_x, pos_y, txt + ": (" + str(x) + "," + str(y) + ")",
              "black",  t, f=('Arial', 14, 'normal'))


def tag_pos(a, b, c):
    # python .\CalcSideLengths.py  5
    # p = (a + b + c) / 2.0
    # s = cmath.sqrt(p * (p - a) * (p - b) * (p - c))
    # y = 2.0 * s / c
    # x = cmath.sqrt(b * b - y * y)

    cos_a = (b * b + c*c - a * a) / (2 * b * c)
    x = b * cos_a
    y = b * cmath.sqrt(1 - cos_a * cos_a)

    return round(x.real, 1), round(y.real, 1)


def uwb_range_offset(uwb_range):

    temp = uwb_range
    return temp


client = mqtt.Client()
mqtt_broker = "raspberrypi.local"
mqtt_topic_uwb = "uwbtag/CB:50:24:0A:C4:80:CB:50"
mqtt_topic_imu = "imutag/CB:50:24:0A:C4:80:CB:50"
mqtt_username = "emqx"
mqtt_password = "brokerpw1"
mqtt_port = 1883


def mqtt_setup():
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set(mqtt_username, mqtt_password)
    client.connect(mqtt_broker, mqtt_port)
    client.loop_start()


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully")
        client.subscribe(mqtt_topic_uwb)
        client.subscribe(mqtt_topic_imu)
    else:
        print("Connect returned result code: " + str(rc))


data_uwb_list = []


def on_message(client, userdata, msg):
    global data_uwb_list
    if msg.topic == mqtt_topic_uwb:
        print("Received UWB Location Update")
        data_uwb = msg.payload.decode("utf-8")
        print(data_uwb)
        try:
            data_uwb_json = json.loads(data_uwb)
            # print(data_uwb_json)
            data_uwb_list = data_uwb_json["links"]
            # for uwb_archor in data_uwb_list:
            #     print(uwb_archor)
        except:
            print("Bad Data")

    elif msg.topic == mqtt_topic_imu:
        print("Received IMU Location Update")
        data_imu = msg.payload.decode("utf-8")
        print(data_imu)
        try:
            # Load IMU data (angular velocity in m/s)
            data_imu_json = json.loads(data_imu)
            acceleration_data = np.array([data_imu_json["IMUEvent"]["Accel"]["X"],
                                        data_imu_json["IMUEvent"]["Accel"]["Y"],
                                        data_imu_json["IMUEvent"]["Accel"]["Z"]])

            # Load gyro data (angular velocity in rad/s)
            gyro_data = np.array([data_imu_json["IMUEvent"]["Gyro"]["X"],
                                data_imu_json["IMUEvent"]["Gyro"]["Y"],
                                data_imu_json["IMUEvent"]["Gyro"]["Z"]])

            # Extract timestamp in milliseconds
            # timestamp_ms = data_imu_json["IMUEvent"]["TimeStamp"]
            # Calculate time intervals between data points based on timestamps (in seconds)
            # time_interval = timestamp_ms / 1000.0  # Convert milliseconds to seconds
            
            # # Calculate the traveled distance
            # total_distance = calculate_distance(acceleration_data, time_interval)
            # print("Total traveled distance:", total_distance, "meters")
            # distance_along_direction = calculate_distance_along_direction(acceleration_data, gyro_data, time_interval)
            # print("Distance along the direction of rotation:", distance_along_direction, "meters")
        except:
            print("Bad Data")
    else:
        print("Uncategorized Topic:", msg.topic)

data_uwb_list = []

url = 'http://raspberrypi.local:8081/deephub/v1'
zone_foreign_id = 'office.c1tech.zone'
provider_id_anchor1_uwb = '2072'
provider_id_anchor2_uwb = '4C3D'
provider_id_tag1_uwb = 'CB50'
trackable_url: str
#
# Check whether a DeepHub instance is available at the given URL.
#


def is_healthy():
    try:
        return rest.get(url + "/health").status_code == 200
    except:
        print('Could not find a DeepHub instance running at', url)
        False
#
# Update the pallet trackable such that it is attached to the provider with the given id.
#


def attach_trackable_to_provider(provider_id: str):
    trackable_pallet = dh.Trackable(rest.get(trackable_url).json())
    trackable_pallet.location_providers = [provider_id]
    rest.put(trackable_url, trackable_pallet.to_json())
#
# Send location updates for the provider with the given id from the given file.
#


def send_location_updates(provider_id: str, provider_type: str, file: str, reverse: bool = False):
    location = dh.Location(provider_id=provider_id,
                           provider_type=provider_type)
    if provider_type == 'gps':
        location.crs = 'EPSG:4326'
    else:
        location.source = zone_foreign_id

    with open(file) as input:
        if reverse:
            input = reversed(list(input))
        for coordinates in [list(map(float, line.split(sep=','))) for line in input]:
            location.position = dh.Point(coordinates=coordinates)
            rest.put(url + '/providers/locations', location.to_json_list())
            time.sleep(0.05)
#
# Initialize all the example entities in the DeepHub.
#


def setup():
    global trackable_url

    # Check whether the office's entities exist already.
    if len(rest.get(url + '/zones' + '?foreign_id=' + zone_foreign_id).json()) > 0:
        print('Found offie zone. Continue.')
        trackable_id_pallet = rest.get(url + '/trackables').json()[0]
        trackable_url = url + '/trackables/' + trackable_id_pallet
        return

    # Setup the office's zone.
    zone = dh.Zone()
    zone.name = 'Area'
    zone.foreign_id = zone_foreign_id
    rest.post(url + '/zones', zone.to_json())

    # Setup the office's fences.
    # delivery_fence = dh.Fence(region=dh.Polygon())
    # delivery_fence.name = 'Delivery'
    # rest.post(url + '/fences', delivery_fence.to_json())

    # drop_fence = dh.Fence(region=dh.Point())
    # drop_fence.name = 'Drop'
    # drop_fence.radius = 2
    # rest.post(url + '/fences', drop_fence.to_json())

    # Setup the office's providers for tag and anchors
    provider_anchor1_uwb = dh.LocationProvider(id=provider_id_anchor1_uwb)
    provider_anchor1_uwb.name = 'UWB Anchor 1'
    provider_anchor1_uwb.type = 'uwb'
    rest.post(url + '/providers', provider_anchor1_uwb.to_json())

    provider_anchor2_uwb = dh.LocationProvider(id=provider_id_anchor2_uwb)
    provider_anchor2_uwb.name = 'UWB Anchor 2'
    provider_anchor2_uwb.type = 'uwb'
    rest.post(url + '/providers', provider_anchor2_uwb.to_json())

    provider_tag1_uwb = dh.LocationProvider(id=provider_id_tag1_uwb)
    provider_tag1_uwb.name = 'UWB Tag 1'
    provider_tag1_uwb.type = 'uwb'
    rest.post(url + '/providers', provider_tag1_uwb.to_json())

    # Setup the example's trackable.
    trackable_pallet = dh.Trackable()
    trackable_pallet.name = 'Pallet'
    trackable_pallet.radius = 1
    response = rest.post(url + '/trackables', trackable_pallet.to_json())
    # Obtain the trackables UUID from the response.
    trackable_id_pallet = response.json()['id']
    trackable_url = url + '/trackables/' + trackable_id_pallet


def setupAnchors():
    # Set Anchors Location
    data = {
        "position": {
            "type": "Point",
            "coordinates": [3.7, 0.9]
        },
        "source": "office.c1tech.zone",
        "provider_type": "uwb",
        "provider_id": "2072"
    }
    json_data = json.dumps(data)
    response = rest.put(url + '/providers/2072/location',  data=json_data)
    print('Response:', response.status_code)

    data = {
        "position": {
            "type": "Point",
            "coordinates": [3.7, 4.9]
        },
        "source": "office.c1tech.zone",
        "provider_type": "uwb",
        "provider_id": "4C3D"
    }
    json_data = json.dumps(data)
    response = rest.put(url + '/providers/4C3D/location',  data=json_data)
    print('Response:', response.status_code)


def send_to_deephub(x, y):
    data = {
        "position": {
            "type": "Point",
            "coordinates": [x, y]
        },
        "source": "office.c1tech.zone",
        "provider_type": "uwb",
        "provider_id": "CB50"
    }
    json_data = json.dumps(data)
    response = rest.put(url + '/providers/CB50/location',  data=json_data)
    print('Tag Location Updated With Response Code:', response.status_code)


# Sample IMU data in JSON format
imu_data_json = '''
{
    "imuEvent": {
        "Accel": {"X": -0.32, "Y": 0.41, "Z": 10.86},
        "Gyro": {"X": -0.03, "Y": 0.03, "Z": -0.05},
        "Temp": 67.20
    }
}
'''


def calculate_distance(acceleration_data, time_intervals):
    velocity = acceleration_data * time_intervals

    # Integrate velocity to obtain displacement (traveled distance)
    displacement = 0.5 * acceleration_data * (time_intervals ** 2)

    # Total traveled distance is the sum of displacement along each axis
    total_distance = np.sum(displacement)

    return total_distance

def calculate_distance_along_direction(acceleration_data, gyro_data, time_interval):
    # Calculate traveled distance using the formula: distance = 0.5 * acceleration * time^2
    total_distance = 0.5 * np.linalg.norm(acceleration_data) * (time_interval ** 2)
    
    # Calculate the rotation using gyro data: rotation = angular_velocity * time
    rotation = np.linalg.norm(gyro_data) * time_interval
    
    # Calculate the distance along the direction of rotation: distance_along_direction = total_distance * cos(rotation)
    distance_along_direction = total_distance * np.cos(rotation)
    
    return distance_along_direction

def main():

    mqtt_setup()
    # t_ui = turtle.Turtle()
    # t_a1 = turtle.Turtle()
    # t_a2 = turtle.Turtle()
    # t_a3 = turtle.Turtle()
    # turtle_init(t_ui)
    # turtle_init(t_a1)
    # turtle_init(t_a2)
    # turtle_init(t_a3)

    a1_range = 0.0
    a2_range = 0.0
    global data_uwb_list

    # draw_ui(t_ui)

    is_healthy()
    print('Setting up the workspace.')
    # setup()
    # setupAnchors()
    print('DeepHub Setup Done.')

    while True:
        node_count = 0
        if client.is_connected:
            if data_uwb_list != []:
                for one in data_uwb_list:
                    if one["A"] == "4C3D":
                        clean(t_a1)
                        a1_range = uwb_range_offset(float(one["R"]))
                        draw_uwb_anchor(-250, 180, "4C3D(0,0)", a1_range, t_a1)
                        node_count += 1

                    if one["A"] == "2072":
                        clean(t_a2)
                        a2_range = uwb_range_offset(float(one["R"]))
                        draw_uwb_anchor(-250 + meter2pixel * distance_a1_a2,
                                        180, "2072(" + str(distance_a1_a2)+")", a2_range, t_a2)
                        node_count += 1

                if node_count == 2:
                    if a2_range != 0 and a1_range != 0:
                        x, y = tag_pos(a2_range, a1_range, distance_a1_a2)
                        print("Tag Position: X=", x, " Y=", y)
                        clean(t_a3)
                        draw_uwb_tag(x, y, "TAG", t_a3)
                        # 1. X and Y are inverted beetwen this code and DeepHub
                        # 2. 4C3D is in the (0,0) postion of this code so we subtract its postion in DeepHub (3.7, 4.9)
                        send_to_deephub(3.7 - y, 4.9 - x)
                        print("")

            data_uwb_list = []
        else:
            print("Waiting for MQTT Connection...")
            time.sleep(5)

        time.sleep(1)

    turtle.mainloop()


if __name__ == '__main__':
    main()
