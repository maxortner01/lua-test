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
                },
                {
                    type = Component.Collider,
                    value = { ColliderComponent = Component.Tilemap }
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
                "/scripts/fps.lua"
            }
        },
        {
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 175, y = 175, z = 1 },
                        scale = 3,
                        rotation = 0
                    }
                },
                {
                    type = Component.Sprite,
                    value = {
                        size = { width = 16, height = 16 },
                        texture = ""
                    }
                },
                {
                    type = Component.Rigidbody,
                    value = {
                        velocity = { x = 425.235235232, y = 295.3255 }
                    }
                },
                {
                    type = Component.Collider,
                    value = { ColliderComponent = Component.Sprite }
                }
            },
            scripts = {
                "/scripts/movement.lua"
            }
        }
    }
end


