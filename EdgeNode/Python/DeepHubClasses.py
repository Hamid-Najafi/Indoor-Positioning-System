import ujson as json

# The default value used for Points in WGS84 coordinates.
default_point = [8.675234, 49.415941]

# The default value used for Polygons in WGS84 coordinates.
default_polygon = [[[8.675755, 49.416015],
                    [8.675852, 49.416015],
                    [8.675853, 49.415916],
                    [8.675755, 49.415916],
                    [8.675755, 49.416015]]]

# The default value used for zone GCPs as pairs of WGS84 coordinates and their respective zone-local coordinates.
default_ground_control_points = [[ 59.50221435502408 ,36.353020375484746], [3.7, 5.8],
                                 [59.5024781781687   ,36.35301044773591],   [3.7, 0.0],
                                 [59.502469007058465 ,36.35279893348107], [0.0, 0.0],
                                 [59.50214086428721  ,36.35281192901468],  [0.0, 7.1],
                                 [59.50215402424868  ,36.35298794148507],  [1.6, 7.1],
                                 [59.50221158781676  ,36.35298624477244],  [1.6, 5.8]]


class JsonSerializable(object):
    def to_json(self) -> str:
        return json.dumps(self, default=lambda x: x.__dict__)

    def to_json_list(self) -> str:
        return '[' + self.to_json() + ']'

    def pretty(self) -> str:
        return json.dumps(self, default=lambda x: x.__dict__, indent=2)


class Point(JsonSerializable):
    def __init__(self, other=None, objType='Point', coordinates=default_point):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.type, self.coordinates = objType, coordinates


class Polygon(JsonSerializable):
    def __init__(self, other=None, objType='Polygon', coordinates=default_polygon):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.type, self.coordinates = objType, coordinates


class Zone(JsonSerializable):
    def __init__(self, other=None):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.id = 'ZoneId'
            self.type = 'uwb'
            self.ground_control_points = default_ground_control_points


class Fence(JsonSerializable):
    def __init__(self, other=None, region=Point()):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.id = 'FenceId'
            self.region = region


class Trackable(JsonSerializable):
    def __init__(self, other=None, polygon=Polygon(), location_providers=[]):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.id = 'TrackableId'
            self.type = 'omlox'
            self.geometry = polygon
            self.location_providers = location_providers


class LocationProvider(JsonSerializable):
    def __init__(self, other=None, id='ProviderId'):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.id = id
            self.type = 'gps'


class Location(JsonSerializable):
    def __init__(self, other=None, provider_id='ProviderId', provider_type='uwb', position=Point()):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.position = position
            self.provider_id = provider_id
            self.provider_type = provider_type
            self.source = provider_id

class Anchor(JsonSerializable):
    def __init__(self, other=None, provider_id='ProviderId', provider_type='uwb', position=Point()):
        if type(other) is dict:
            for key, value in other.items():
                self.__dict__[key] = value
        else:
            self.position = position
            self.provider_id = provider_id
            self.provider_type = provider_type
            self.source = provider_id