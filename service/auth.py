import hashlib
secret_key = "mx72a7KRgiLod0rkWVIwMrknZ1MW38gb302BYItfE3vg3Lz62vDtqWACXl77Szmo"

# 计算默认不加盐的md5
def get_md5(sign_str):
    md5_hash = hashlib.md5()
    md5_hash.update(sign_str.encode('utf-8'))
    return md5_hash.hexdigest()

# 规则&&链接 （时间戳字符串&&密钥&&时间戳字符串）生成md5的签名sign
def generate_sign(secret_key, timestamp):
    sign_str = "&&".join([str(timestamp), secret_key, str(timestamp)])
    sign = get_md5(sign_str)
    return sign