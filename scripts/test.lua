player = {}

function AddStuff(a, b)
    return a + b
end

function Start()
    player["Level"] = 20
    player["Name"] = "Max"

    Debug.log("Created player with name '"..player["Name"].."' and at level "..player["Level"])
    Debug.assert(player["Level"] == 21)
end

function Update()
    player["Level"] = AddStuff(player["Level"], 1)
    print(player["Level"])
end