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
        }
    }
end

function GetEntities()
    return {
        {
            name = "PlayerTop",
            components = {
                {
                    type = Component.Transform,
                    value = {
                        position = { x = 0, y = 0, z = 2 },
                        scale = 0.25,
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
                        linearDrag = 3
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
                        position = { x = 0, y = 0, z = 2 },
                        scale = 0.25,
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
                        position = { x = 0, y = 0, z = 15 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 90,
                        projection = ProjectionType.Orthographic,
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
                        position = { x = 0, y = 0, z = 15 },
                        scale = 1,
                        rotation = 0
                    }
                },
                {
                    type = Component.Camera,
                    value = {
                        FOV = 90,
                        projection = ProjectionType.Orthographic,
                        size = { width = 1280, height = 720 }
                    }
                }
            }
        }
    }
end