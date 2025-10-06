export interface ImageItem {
  id: string
  imageUrl: string
  title: string
  userAvatar: string
  userId: string
  likeCount: number
  isLiked: boolean
  imageHeight: number
}

@Component
export struct ImageCard {
  @Prop item: ImageItem
  onLikeClick?: (id: string) => void

  build() {
    Column() {
      Image(this.item.imageUrl)
        .width('100%')
        .height(this.item.imageHeight)
        .objectFit(ImageFit.Cover)
        .borderRadius({ topLeft: 8, topRight: 8 })

      Column() {
        Text(this.item.title)
          .fontSize(14)
          .fontColor('#333333')
          .maxLines(2)
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .width('100%')
          .margin({ bottom: 8 })

        Row() {
          Row() {
            Image(this.item.userAvatar)
              .width(20)
              .height(20)
              .borderRadius(10)
              .margin({ right: 6 })

            Text(this.item.userId)
              .fontSize(12)
              .fontColor('#666666')
          }
          .layoutWeight(1)

          Row() {
            Text(this.item.isLiked ? '♥' : '♡')
              .fontSize(16)
              .fontColor(this.item.isLiked ? '#FF4757' : '#999999')
              .onClick(() => {
                if (this.onLikeClick) {
                  this.onLikeClick(this.item.id)
                }
              })

            Text(this.item.likeCount.toString())
              .fontSize(12)
              .fontColor('#666666')
              .margin({ left: 4 })
          }
        }
        .width('100%')
        .alignItems(VerticalAlign.Center)
      }
      .padding(12)
      .width('100%')
    }
    .backgroundColor('#FFFFFF')
    .borderRadius(8)
    .width('100%')
  }
}


