```json
{
    "timestamp": 1697510738,
    "version": "v1.0",
    "sign": "",
    "data": {
      "task_name": "My Task",
      "start_time": "2022-01-01 09:00:00",
      "end_time": "2022-01-01 18:00:00",
      "video_type": 1,
      "sources": [
        {
          "video_type": "webm",
          "main_video": "http://example.com/main_video.mp4",
          "mask_video": "http://example.com/mask_video.mp4",
          "bg_image": {
            "url": "http://example.com/bg_image.jpg",
            "size": "800x600",
            "level": 0,
          },
          "patches": [
            {
            "url": "http://example.com/bg_image.jpg",
            "size": "800x600",
            "level": 0,
            "x": 0,
            "y": 0
          },
           {
            "url": "http://example.com/bg_image.jpg",
            "size": "800x600",
            "level": 0,
            "x": 0,
            "y": 0
          },
          ],
          "close_shot": {
            "url": "http://example.com/close_shot.mp4",
            "level": 2
          },
          "bg_video": {
            "url": "http://example.com/bg_video.mp4"
          }
        }
      ],
      "callback_url": "http://example.com/callback",
      "rtmp": "rtmp://example.com/stream",
      "ext": {}
    }
  }
  
```

```json
{
    "code": 200,
    "timestamp": 1697510738,
    "data": {
        "task_name": "My Task",
        "task_id": "1234567890"
    },
    "message": "Task completed successfully"
}
```
