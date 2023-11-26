function GetResources()
    return {
        textures = {
            {
                name = "tilemap",
                location = Directory.Source.."/textures/GRASS+.png"
            },
            {
                name = "mask",
                location = Directory.Source.."/textures/grass_mask.png"
            }
        },
        fonts = {
            {
                name = "arial",
                location = Directory.Source.."/fonts/arial.ttf"
            }
        },
        images = {
            {
                name = "mask",
                location = Directory.Source.."/textures/grass_mask.png"
            }
        },
        shaders = {
            {
                name = "test",
                vertex   = Directory.Source.."/shaders/vertex.glsl",
                fragment = Directory.Source.."/shaders/fragment.glsl"
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
                Directory.Source.."/scripts/yard.lua"
            }
        },
        {
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 100 + 20 * 16 * 3 / 2.0, y = 100 + 20 * 16 * 3 / 2.0, z = 0 },
                        scale = 3,
                        rotation = 0
                    }
                },
                {
                    type = Component.Sprite,
                    value = {
                        size = { width = 16 * 20, height = 16 * 20 },
                        texture = "mask"
                    }
                }
            }
        },
        {
            name = "Mesh",
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
                    type = Component.CustomMesh,
                    value = {}
                }
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
                        linearDrag = 10
                    }
                },
                {
                    type = Component.Collider,
                    value = { ColliderComponent = Component.Sprite }
                }
            },
            scripts = {
                Directory.Source.."/scripts/mower.lua"
            }
        },
        {
            name = "MainCamera",
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
                        projection = ProjectionType.Orthographic
                    }
                }
            },
            scripts = {
                Directory.Source.."/scripts/camera.lua"
            }
        },
        {
            name = "MowCamera",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 320 * 3 / 2.0, y = 320 * 3 / 2.0, z = 0 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 90,
                        projection = ProjectionType.Orthographic
                    }
                }
            }
        }
    }
end


