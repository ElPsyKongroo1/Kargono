import openai

# System gives chatgpt information about their role
# User specifies infomation from the end user (me)

openai.api_key = "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"

response = openai.ChatCompletion.create(
    model="gpt-3.5-turbo",
    messages = [
        {"role": "system", "content": "The format of your response"
                                      " with key-value pair dictionary elements."},
        {"role": "user", "content": "Give me the ID number 23 and the name 'Charles'"}

    ]
)
print(response)
print(response.choices[0].message.content)