player = {}

function AddStuff(a, b)
    return a + b
end

function Start()
    player["Level"] = 20
    player["Name"] = "Max"
end

function Update()
    player["Level"] = AddStuff(player["Level"], 1)
    print(player["Level"])
end