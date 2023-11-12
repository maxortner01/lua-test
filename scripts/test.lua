Player = {}

function Start()
    Player.Level = 20
    Player.Name  = "Max"

    -- Debug library has logging built into it
    Debug.log("Created Player with namejhgj '"..Player.Name.."' and at level "..Player.Level)
    Debug.assert(Player.Level == 21)

    -- We can also retreive structs as Lua tables pretty easily
    local data = Debug.getTable()
    Debug.assert(data.good)
    Debug.log("coolness = "..data.coolness)

    -- There is also support for passing lua tables as structs, as well as overload resolution
    local pos = {}
    pos.x = 2.0
    pos.y = 4.0
    Debug.log("Position = ("..pos.x..", "..pos.y..") with a distance of "..Debug.distance(pos))
end

function Update()
    print(Player.Level)
end