from fastapi import FastAPI, Request
import uvicorn
import requests

app = FastAPI()


# 添加在应用程序启动之前运行的函数
@app.on_event("startup")
async def startup_event():
    print("关闭应用程序啦")
    with open("log.txt", mode="a") as log:
        log.write("Application startup")


# 添加在应用程序关闭时运行的函数
@app.on_event("shutdown")
async def shutdown_event():
    print("关闭应用程序啦")
    with open("log.txt", mode="a") as log:
        log.write("Application shutdown")


@app.post("/api/v1/create_task")
def create_task(request: Request):
    data = request.json()
    print(data)
    return {"message": "Task created"}


@app.post("/api/v1/update_task")
def create_task(request: Request):
    data = request.json()
    print(data)
    return {"message": "Task created"}


@app.post("/api/v1/delete_task")
def create_task(request: Request):
    data = request.json()
    print(data)
    return {"message": "Task created"}


@app.post("/api/v1/close_task ")
def create_task(request: Request):
    data = request.json()
    print(data)
    return {"message": "Task created"}


@app.post("/api/v1/query_task")
def create_task(request: Request):
    data = request.json()
    print(data)
    return {"message": "Task created"}


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
