function GetResources()
    return {
        textures = {
            {
                name = "tilemap",
                location = "/textures/Dungeon_Tileset.png"
            }
        },
        fonts = {
            {
                name = "arial",
                location = "/fonts/arial.ttf"
            }
        }
    }
end

function GetEntities()
    return {
        {
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 100, y = 100, z = 0 },
                        scale = 3,
                        rotation = 0
                    }
                },
                {
                    type = Component.Tilemap,
                    value = {
                        spritesheet = { texture_name = "tilemap" },
                        tilesize = { width = 16, height = 16 }
                    }
                }
            },
            scripts = {
                "/scripts/tilemap.lua"
            }
        },
        {
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 10, y = 20, z = 0 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Text,
                    value = {
                        font = "arial",
                        string = "",
                        characterSize = 16,
                        textAlign = 0
                    }
                }
            },
            scripts = {
                "/scripts/fps.lua",
                "/scripts/movement.lua"
            }
        }
    }
end


