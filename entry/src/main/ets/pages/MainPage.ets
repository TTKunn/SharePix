import { FollowPage } from '../views/pages/FollowPage'
import { DiscoverPage } from '../views/pages/DiscoverPage'
import { MessagePage } from '../views/pages/MessagePage'
import { ProfilePage } from '../views/pages/ProfilePage'

@Entry
@Component
struct MainPage {
  @State currentIndex: number = 1

  build() {
    Tabs({ barPosition: BarPosition.End, index: this.currentIndex }) {
      TabContent() {
        FollowPage()
      }
      .tabBar(this.TabBuilder('关注', 0))

      TabContent() {
        DiscoverPage()
      }
      .tabBar(this.TabBuilder('发现', 1))

      TabContent() {
        MessagePage()
      }
      .tabBar(this.TabBuilder('消息', 2))

      TabContent() {
        ProfilePage()
      }
      .tabBar(this.TabBuilder('我的', 3))
    }
    .onChange((index: number) => {
      this.currentIndex = index
    })
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }

  @Builder TabBuilder(title: string, targetIndex: number) {
    Column() {
      Text(title)
        .fontColor(this.currentIndex === targetIndex ? '#007DFF' : '#999999')
        .fontSize(12)
        .fontWeight(this.currentIndex === targetIndex ? FontWeight.Bold : FontWeight.Normal)
    }
    .width('100%')
    .height(56)
    .justifyContent(FlexAlign.Center)
  }
}
