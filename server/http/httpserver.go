package http

import(
    "github.com/gin-gonic/gin"
)

func NewRouter() *gin.Engine{
    router := gin.New()
    router.Use(gin.Logger())
    router.Use(CORSMiddleware())

    router.GET("/logs")
    router.GET("/users")
    router.POST("/auth")

    return router
}

