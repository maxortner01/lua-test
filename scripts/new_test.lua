function GetResources()
    return {
        textures = {
            {
                name = "mower_top",
                location = Directory.Source.."/textures/mower_top.png"
            },
            {
                name = "mower_bottom",
                location = Directory.Source.."/textures/mower_bottom.png"
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
                name = "grass_shader",
                vertex   = Directory.Source.."/shaders/vertex.glsl",
                fragment = Directory.Source.."/shaders/fragment.glsl"
            },
            {
                name = "bottom",
                vertex   = Directory.Source.."/shaders/bottom.vert",
                fragment = Directory.Source.."/shaders/bottom.frag"
            }
        },
        fonts = {
            {
                name = "arial",
                location = Directory.Source.."/fonts/Baskervald-Italic.otf"
            }
        }
    }
end

function GetEntities()
    return {
        {
            name = "Mesh",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = -0.1 },
                        scale = 16,
                        rotation = 0
                    }
                },
                {
                    type = Component.CustomMesh,
                    value = {}
                },
                {
                    type = Component.Shader,
                    value = {
                        name = "grass_shader"
                    }
                }
            }
        },
        {
            name = "PlayerTop",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = 0 },
                        scale = 1.0,
                        rotation = 0
                    }
                },
                {
                    type = Component.Sprite,
                    value = {
                        size = { width = 24.0 / 24.0, height = 42.0 / 24.0 },
                        texture = "mower_top"
                    }
                },
                {
                    type = Component.Rigidbody,
                    value = {
                        velocity   = { x = 0, y = 0, z = 0 },
                        addedForce = { x = 0, y = 0, z = 0 },
                        linearDrag = 8
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
            name = "PlayerBottom",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = 0 },
                        scale = 1.0,
                        rotation = 0
                    }
                },
                {
                    type = Component.Sprite,
                    value = {
                        size = { width = 24 / 24.0, height = 42 / 24.0 },
                        texture = "mower_bottom"
                    }
                }
            },
            scripts = {
                Directory.Source.."/scripts/mower_bottom.lua"
            }
        },
        {
            name = "MainCamera",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = 3.7 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 100,
                        projection = ProjectionType.Perspective,
                        size = { width = 1280, height = 720 }
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
                        position = { x = 0, y = 0, z = 20 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 100,
                        projection = ProjectionType.Perspective,
                        size = { width = 1000, height = 1000 }
                    }
                }
            }
        }
    }
end