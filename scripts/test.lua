function GetResources()
    return {
        textures = {
            {
                name = "tilemap",
                location = Directory.Source.."/textures/Dungeon_Tileset.png"
            }
        },
        fonts = {
            {
                name = "arial",
                location = Directory.Source.."/fonts/arial.ttf"
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
                Directory.Source.."/scripts/tilemap.lua"
            }
        },
        {
            name = "Player",
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
                        velocity   = { x = 0, y = 0 },
                        addedForce = { x = 0, y = 0 },
                        linearDrag = 0.5
                    }
                },
                {
                    type = Component.Collider,
                    value = { ColliderComponent = Component.Sprite }
                }
            },
            scripts = {
                Directory.Source.."/scripts/movement.lua",
                Directory.Source.."/scripts/combat.lua"
            }
        },
        {
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = 0 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 90,
                        projection = 0
                    }
                }
            },
            scripts = {
                Directory.Source.."/scripts/camera.lua"
            }
        }
    }
end


